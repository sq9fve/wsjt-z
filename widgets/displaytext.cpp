#include "displaytext.h"

#include <vector>
#include <algorithm>

#include <QMouseEvent>
#include <QDateTime>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextBlock>
#include <QMenu>
#include <QAction>
#include <QListIterator>
#include <QRegularExpression>
#include <QScrollBar>

#include "Configuration.hpp"
#include "Decoder/decodedtext.h"
#include "Network/LotWUsers.hpp"
#include "models/DecodeHighlightingModel.hpp"
#include "logbook/logbook.h"
#include "Logger.hpp"

#include "qt_helpers.hpp"
#include "moc_displaytext.cpp"

// Regexes compiled once and reused across every decoded-line render. Constructing
// QRegularExpression on each call of displayDecodedText() measurably dominates the
// display hot path on busy FT8 bands.
static QRegularExpression const kReGrid {
    QStringLiteral("\\A(?![Rr]{2}73)[A-Ra-r]{2}[0-9]{2}([A-Xa-x]{2}){0,1}\\z")};
static QRegularExpression const kReRemoveExtra {QStringLiteral(R"(\? a[1-9]|a[1-9])")};
static QRegularExpression const kReAPPos {
    QStringLiteral(R"((?:\?\s)?(?:a[0-9]|q[0-9][0-9*]?)$)")};

DisplayText::DisplayText(QWidget *parent)
  : QTextEdit(parent)
  , m_config {nullptr}
  , erase_action_ {new QAction {tr ("&Erase"), this}}
  , high_volume_ {false}
  , modified_vertical_scrollbar_max_ {-1}
  , isBandActivity {false}
  , m_freezeUpdates {false}
{
  setReadOnly (true);
  setUndoRedoEnabled (false);
  setMouseTracking(true);
  viewport ()->setCursor (Qt::ArrowCursor);
  setWordWrapMode (QTextOption::NoWrap);

  // max lines to limit heap usage
  document ()->setMaximumBlockCount (5000);

  // context menu erase action
  setContextMenuPolicy (Qt::CustomContextMenu);
  connect (this, &DisplayText::customContextMenuRequested, [this] (QPoint const& position) {
      auto * menu = createStandardContextMenu (position);
      menu->addAction (erase_action_);
      menu->exec (mapToGlobal (position));
      delete menu;
    });
  connect (erase_action_, &QAction::triggered, this, &DisplayText::erase);
}

void DisplayText::erase ()
{
  clear ();
  Q_EMIT erased ();
}

void DisplayText::setContentFont(QFont const& font)
{
  char_font_ = font;
  selectAll ();
  auto cursor = textCursor ();
  cursor.beginEditBlock ();
  auto char_format = cursor.charFormat ();
  char_format.setFont (char_font_);
  cursor.mergeCharFormat (char_format);
  cursor.clearSelection ();
  cursor.movePosition (QTextCursor::End);

  // position so viewport scrolled to left
  cursor.movePosition (QTextCursor::Up);
  cursor.movePosition (QTextCursor::StartOfLine);
  cursor.endEditBlock ();

  if (!high_volume_ || !m_config || !m_config->decodes_from_top ())
    {
      setTextCursor (cursor);
      ensureCursorVisible ();
    }
}

void DisplayText::mouseDoubleClickEvent(QMouseEvent *e)
{
  Q_EMIT selectCallsign(e->modifiers ());
}
 // Z
 void DisplayText::mousePressEvent(QMouseEvent *mouseEvent) {
     QTextCursor textCursor = cursorForPosition(mouseEvent->pos());
     if (Qt::RightButton == mouseEvent->button()) {
         textCursor.select(QTextCursor::LineUnderCursor);
         setTextCursor(textCursor);
     } else {
         textCursor.select(QTextCursor::LineUnderCursor);
         setTextCursor(textCursor);
         Q_EMIT leftClick(mouseEvent->modifiers ());
     }
 }

void DisplayText::insertLineSpacer(QString const& line)
{
  // Z
  insertText (line, m_config->separatorColor());
}

namespace
{
  using Highlight = DecodeHighlightingModel::Highlight;
  using highlight_types = std::vector<Highlight>;
  Highlight set_colours (Configuration const * config, QColor * bg, QColor * fg, highlight_types const& types)
  {
    Highlight result = Highlight::CQ;
    if (config)
      {
        QListIterator<DecodeHighlightingModel::HighlightInfo> it {config->decode_highlighting ().items ()};
        // iterate in reverse to honor priorities
        it.toBack ();
        while (it.hasPrevious ())
          {
            auto const& item = it.previous ();
            auto const& type = std::find (types.begin (), types.end (), item.type_);
            if (type != types.end () && item.enabled_)
              {
                if (item.background_.style () != Qt::NoBrush)
                  {
                    *bg = item.background_.color ();
                  }
                if (item.foreground_.style () != Qt::NoBrush)
                  {
                    *fg = item.foreground_.color ();
                  }
                result = item.type_;
              }
          }
      }
    return result;            // highest priority enabled highlighting
  }
}

void DisplayText::insertText(QString const& text, QColor bg, QColor fg
                             , QString const& call1, QString const& call2, QTextCursor::MoveOperation location, bool bold)
{

    if (m_freezeUpdates) {
        m_updateBuffer.append({text, bg, fg, call1, call2, location, bold});
        return;
    }

  auto cursor = textCursor ();
  cursor.movePosition (location);
  auto block_format = cursor.blockFormat ();
  auto format = cursor.blockCharFormat ();
  format.setFont (char_font_);
  if (bold) {
    format.setFontWeight(QFont::Bold);
  }
  block_format.clearBackground ();
  if (bg.isValid ())
    {
      block_format.setBackground (bg);
    }
  format.clearForeground ();
  if (fg.isValid ())
    {
      format.setForeground (fg);
    }
  if (cursor.position ())
    {
      cursor.insertBlock (block_format, format);
    }
  else
    {
      cursor.setBlockFormat (block_format);
      cursor.setBlockCharFormat (format);
    }

  int text_index {0};
  auto temp_format = format;
  if (call1.size ())
    {
      auto call_index = text.indexOf (call1);
      if (call_index != -1) // sanity check
        {
          auto pos = highlighted_calls_.find (call1);
          if (pos != highlighted_calls_.end ())
            {
              cursor.insertText(text.left (call_index));
              if (pos.value ().first.isValid ())
                {
                  temp_format.setBackground (pos.value ().first);
                }
              if (pos.value ().second.isValid ())
                {
                  temp_format.setForeground (pos.value ().second);
                }
              cursor.insertText(text.mid (call_index, call1.size ()), temp_format);
              text_index = call_index + call1.size ();
            }
        }
    }
  if (call2.size ())
    {
      auto call_index = text.indexOf (call2, text_index);
      if (call_index != -1) // sanity check
        {
          auto pos = highlighted_calls_.find (call2);
          if (pos != highlighted_calls_.end ())
            {
              temp_format = format;
              cursor.insertText(text.mid (text_index, call_index - text_index), format);
              if (pos.value ().second.isValid ())
                {
                  temp_format.setBackground (pos.value ().first);
                }
              if (pos.value ().second.isValid ())
                {
                  temp_format.setForeground (pos.value ().second);
                }
              cursor.insertText(text.mid (call_index, call2.size ()), temp_format);
              text_index = call_index + call2.size ();
            }
        }
    }
  cursor.insertText(text.mid (text_index), format);

  // position so viewport scrolled to left
  cursor.movePosition (QTextCursor::StartOfLine);
  if (!high_volume_ || !m_config || !m_config->decodes_from_top ())
    {
      setTextCursor (cursor);
      ensureCursorVisible ();
    }
  document ()->setMaximumBlockCount (document ()->maximumBlockCount ());
}

void DisplayText::extend_vertical_scrollbar (int min, int max)
{
  if (high_volume_ && m_config && m_config->decodes_from_top ())
    {
      if (max && max != modified_vertical_scrollbar_max_)
        {
          setViewportMargins (0,4,0,0);  // ensure first line is readable
          auto vp_margins = viewportMargins ();
          // add enough to vertical scroll bar range to allow last
          // decode to just scroll of the top of the view port
          max += viewport ()->height () - vp_margins.top () - vp_margins.bottom ();
          modified_vertical_scrollbar_max_ = max;
        }
      verticalScrollBar ()->setRange (min, max);
    }
}

void DisplayText::new_period ()
{
  if (m_config->decodes_from_top ()) {
    document ()->setMaximumBlockCount (4800);
    document ()->setMaximumBlockCount (5000);
  }
  extend_vertical_scrollbar (verticalScrollBar ()->minimum (), verticalScrollBar ()->maximum ());
  if (high_volume_ && m_config && m_config->decodes_from_top () && !vertical_scroll_connection_)
    {
      vertical_scroll_connection_ = connect (verticalScrollBar (), &QScrollBar::rangeChanged
                                             , [this] (int min, int max) {
                                               extend_vertical_scrollbar (min, max );
                                             });
    }
  // Z
  if (!m_freezeUpdates)
  verticalScrollBar ()->setSliderPosition (verticalScrollBar ()->maximum ());
}

QString DisplayText::appendWorkedB4 (QString message, QString call, QString const& grid,
                                     QColor * bg, QColor * fg, LogBook const& logBook,
                                     QString const& currentBand, QString const& currentMode,
                                     QString extra)
{
  QString countryName;
  bool callB4;
  bool callB4onBand;
  bool countryB4;
  bool countryB4onBand;
  bool gridB4;
  bool gridB4onBand;
  bool continentB4;
  bool continentB4onBand;
  bool CQZoneB4;
  bool CQZoneB4onBand;
  bool ITUZoneB4;
  bool ITUZoneB4onBand;

  if(call.length()==2) {
    int i0=message.indexOf("CQ "+call);
    call=message.mid(i0+6,-1);
    i0=call.indexOf(" ");
    call=call.mid(0,i0);
  }
  if(call.length()<3) return message;
  if(!call.contains(QRegExp("[0-9]|[A-Z]"))) return message;

  auto const& looked_up = logBook.countries ()->lookup (call);
  logBook.match (call, currentMode, grid, looked_up, callB4, countryB4, gridB4, continentB4, CQZoneB4, ITUZoneB4);
  logBook.match (call, currentMode, grid, looked_up, callB4onBand, countryB4onBand, gridB4onBand,
                 continentB4onBand, CQZoneB4onBand, ITUZoneB4onBand, currentBand);
  if(grid=="") {
    gridB4=true;
    gridB4onBand=true;
  }

  if(callB4onBand) m_points=0;

  message = message.trimmed ();

  highlight_types types;
  // no shortcuts here as some types may be disabled
  if (!countryB4) {
    types.push_back (Highlight::DXCC);
  }
  if(!countryB4onBand) {
    types.push_back (Highlight::DXCCBand);
  }
  if(!gridB4) {
    types.push_back (Highlight::Grid);
  }
  if(!gridB4onBand) {
    types.push_back (Highlight::GridBand);
  }
  if (!callB4) {
    types.push_back (Highlight::Call);
  }
  if(!callB4onBand) {
    types.push_back (Highlight::CallBand);
  }
  if (!continentB4) {
    types.push_back (Highlight::Continent);
  }
  if(!continentB4onBand) {
    types.push_back (Highlight::ContinentBand);
  }
  if (!CQZoneB4) {
    types.push_back (Highlight::CQZone);
  }
  if(!CQZoneB4onBand) {
    types.push_back (Highlight::CQZoneBand);
  }
  if (!ITUZoneB4) {
    types.push_back (Highlight::ITUZone);
  }
  if(!ITUZoneB4onBand) {
    types.push_back (Highlight::ITUZoneBand);
  }
  if (m_config && m_config->lotw_users ().user (call))
    {
      types.push_back (Highlight::LotW);
    }
  types.push_back (Highlight::CQ);
  auto top_highlight = set_colours (m_config, bg, fg, types);

  switch (top_highlight)
    {
    case Highlight::Continent:
    case Highlight::ContinentBand:
      extra += AD1CCty::continent (looked_up.continent);
      break;
    case Highlight::CQZone:
    case Highlight::CQZoneBand:
      extra += QString {"CQ Zone %1"}.arg (looked_up.CQ_zone);
      break;
    case Highlight::ITUZone:
    case Highlight::ITUZoneBand:
      extra += QString {"ITU Zone %1"}.arg (looked_up.ITU_zone);
      break;
    default:
      if (m_bPrincipalPrefix)
        {
          extra += looked_up.primary_prefix;
        }
      else
        {
          auto countryName = looked_up.entity_name;

          // do some obvious abbreviations
          countryName.replace ("Islands", "Is.");
          countryName.replace ("Island", "Is.");
          countryName.replace ("North ", "N. ");
          countryName.replace ("Northern ", "N. ");
          countryName.replace ("South ", "S. ");
          countryName.replace ("East ", "E. ");
          countryName.replace ("Eastern ", "E. ");
          countryName.replace ("West ", "W. ");
          countryName.replace ("Western ", "W. ");
          countryName.replace ("Central ", "C. ");
          countryName.replace (" and ", " & ");
          countryName.replace ("Republic", "Rep.");
          countryName.replace ("United States of America", "U.S.A.");
          countryName.replace ("United States", "U.S.A.");
          countryName.replace ("Fed. Rep. of ", "");
          countryName.replace ("French ", "Fr.");
          countryName.replace ("Asiatic", "AS");
          countryName.replace ("European", "EU");
          countryName.replace ("African", "AF");

          // assign WAE entities to the correct DXCC when "Include extra WAE entities" is not selected
          if (!(m_config->include_WAE_entities())) {
            countryName.replace ("Bear Is.", "Svalbard");
            countryName.replace ("Shetland Is.", "Scotland");
            countryName.replace ("AF Italy", "Italy");
            countryName.replace ("Sicily", "Italy");
            countryName.replace ("Vienna Intl Ctr", "Austria");
            countryName.replace ("AF Turkey", "Turkey");
            countryName.replace ("EU Turkey", "Turkey");
          }

          extra += countryName;
        }
    }
    m_CQPriority=DecodeHighlightingModel::highlight_name(top_highlight);

    if(((m_points == 00) or (m_points == -1)) and m_bDisplayPoints) return message;
    return leftJustifyAppendage (message, extra);
}

QString DisplayText::leftJustifyAppendage (QString message, QString const& appendage0) const
{
  QString appendage=appendage0;
  if(m_bDisplayPoints and (m_points>0)) {
    appendage=" " + QString::number(m_points);
    if(m_points<10) appendage=" " + appendage;
  }
  if (appendage.size ())
    {
      // allow for seconds
      int padding {message.indexOf (" ") > 4 ? 2 : 0};

      // use a nbsp to save the start of appended text so we can find
      // it again later, align appended data at a fixed column if
      // there is space otherwise let it float to the right
      int space_count {m_config->padding() + padding - message.size ()};
      if (space_count > 0) {
        message += QString {space_count, QChar {' '}};
      }
      message += QChar::Nbsp + appendage;
    }
  return message;
}

void DisplayText::displayDecodedText(DecodedText const& decodedText, QString const& myCall,
                                     QString const& mode,
                                     bool displayDXCCEntity, LogBook const& logBook,
                                     QString const& currentBand, bool ppfx, bool bCQonly,
                                     bool haveFSpread, float fSpread, bool bDisplayPoints, int points,
                                     bool incl73, bool colourAll, QString distance, QString state, bool filtered)
{
  m_points=points;
  m_bDisplayPoints=bDisplayPoints;
  m_bPrincipalPrefix=ppfx;
  QColor bg;
  QColor fg;
  bool CQcall = false;

	// Z 
  qDebug() << "incl73 value:" << incl73;
	
  auto is_73 = decodedText.messageWords().filter (QRegularExpression {"^(73|RR73)$"}).size();
  if (decodedText.string ().contains (" CQ ")
      || decodedText.string ().contains (" CQDX ")
      || decodedText.string ().contains (" QRZ ")
      || (incl73 && is_73))
    {
      CQcall = true;
    }

  auto for_us = (myCall != "" && (decodedText.indexOf (" " + myCall + " ") >= 0
                        or decodedText.indexOf (" " + myCall + "/") >= 0
                        or decodedText.indexOf ("<" + myCall + "/") >= 0
                        or decodedText.indexOf ("/" + myCall + " ") >= 0
                        or decodedText.indexOf ("/" + myCall + ">") >= 0
                        or decodedText.indexOf ("<" + myCall + " ") >= 0
                        or decodedText.indexOf ("<" + myCall + ">") >= 0
                        or decodedText.indexOf (" " + myCall + ">") >= 0));

  if (bCQonly && !CQcall && !for_us) return;
  auto message = decodedText.string();
  QString dxCall;
  QString dxGrid;
  decodedText.deCallAndGrid (/*out*/ dxCall, dxGrid);
  if(!dxGrid.contains(kReGrid)) dxGrid="";
  message = message.left (message.indexOf (QChar::Nbsp)).trimmed ();

  // Z
  if (m_config->removeExtra()) {
      message.replace(kReRemoveExtra, "");
      message = message.trimmed();
  }

  QString extra;
  if (haveFSpread)
    {
      extra += QString {"%1"}.arg (fSpread, 5, 'f', fSpread < 0.95 ? 3 : 2) + QChar {' '};
    }
  auto ap_pos = message.lastIndexOf (kReAPPos);
  if (ap_pos >= 0)
    {
      extra += message.mid (ap_pos) + QChar {' '};
      message = message.left (ap_pos).trimmed ();
    }
  m_CQPriority="";
  if (CQcall || colourAll ||  (is_73 && (m_config->highlight_73 ())))
    {
      if (displayDXCCEntity)
        {
          // if enabled add the DXCC entity and B4 status to the end of the
          // preformated text line t1
          auto currentMode = mode;
          message = appendWorkedB4 (message, dxCall, dxGrid, &bg, &fg
                                    , logBook, currentBand, currentMode, extra);
        }
      else
        {
          message = leftJustifyAppendage (message, extra);
          highlight_types types {Highlight::CQ};
          if (m_config && m_config->lotw_users ().user (decodedText.CQersCall()))
            {
              types.push_back (Highlight::LotW);
            }
          set_colours (m_config, &bg, &fg, types);
        }
    }
  else
    {
      message = leftJustifyAppendage (message, extra);
    }

  if (for_us) {
     highlight_types types {Highlight::MyCall};
     set_colours (m_config, &bg, &fg, types);
   }

  if (state.length() > 0) {
      message = leftJustifyAppendage (message, "[" + state + "]");
  }

  if (distance.length() > 0) {
      message = leftJustifyAppendage (message, "[" + distance + "]");
  }

  if (filtered)
        message = leftJustifyAppendage (message, "[F]");

  insertText (message.trimmed (), bg, fg, decodedText.call (), dxCall);
}


void DisplayText::displayTransmittedText(QString text, QString modeTx, qint32 txFreq,
                                         bool bFastMode, double TRperiod,bool bSuperfox)
{
    QString t1=" @  ";
    if(modeTx=="FT4") t1=" +  ";
    if(modeTx=="FT2") t1=" *  ";
    if(modeTx.contains("FT8")) t1=" ~  ";
    if(modeTx=="JT4") t1=" $  ";
    if(modeTx=="Q65") t1=" :  ";
    if(modeTx=="JT65") t1=" #  ";
    if(modeTx=="MSK144") t1=" &  ";
    if(modeTx=="FST4") t1=" `  ";
    QString t2;
    t2 = t2.asprintf("%4d",txFreq);
    QString t;
    if(bFastMode or modeTx=="FT8" or modeTx=="FT4" or (TRperiod<60) or
       (modeTx=="Q65" and TRperiod==60)) {
      t = QDateTime::currentDateTimeUtc().toString("hhmmss") + \
        "  Tx      " + t2 + t1 + text;
    } else if(modeTx.mid(0,6)=="FT8fox") {
      t = QDateTime::currentDateTimeUtc().toString("hhmmss") + \
        " Tx" + modeTx.mid(7) + " " + text;
    } else {
      t = QDateTime::currentDateTimeUtc().toString("hhmm") + \
        "  Tx      " + t2 + t1 + text;
    }
    QColor bg;
    QColor fg;
    highlight_types types {Highlight::Tx};
    set_colours (m_config, &bg, &fg, types);
    if(bSuperfox and t.contains(";")) {
      int i0=t.indexOf(";");
      int i1=t.indexOf("<");
      int i2=t.indexOf(">");
      QString foxcall=t.mid(i1+1,i2-i1-1);
      t2=t.left(i0).replace(" RR73", " " + foxcall + " RR73");
      QString t3=t.left(24) + t.mid(i0+2,-1).remove("<").remove(">");
      insertText (t2, bg, fg);
      insertText (t3, bg, fg);
    } else {
      insertText (t, bg, fg);
    }
}

void DisplayText::displayQSY(QString text)
{
  QString t = QDateTime::currentDateTimeUtc().toString("hhmmss") + "            " + text;
  insertText (t, "hotpink");
}

void DisplayText::displayHoundToBeCalled(QString t, bool bAtTop, QColor bg, QColor fg)
{
  if (bAtTop)  t = t + "\n"; // need a newline when insertion at top
  insertText(t, bg, fg, "", "", bAtTop ? QTextCursor::Start : QTextCursor::End);
}

void DisplayText::setHighlightedHoundText(QString t) {
  QColor bg=QColor{255,255,255};
  QColor fg=QColor{0,0,0};
  highlight_types types{Highlight::Call};
  set_colours(m_config, &bg, &fg, types);
  // t is multiple lines of text, each line is a hound calling
  // iterate through each line and highlight the callsign
  auto lines = t.split(QChar('\n'), SkipEmptyParts);
  clear();
  foreach (auto line, lines)
  {
    auto fields = line.split(QChar(' '), SkipEmptyParts);
    insertText(line, bg, fg, fields.first(), QString{});
  }
}

namespace {
  QString get_timestamp(QTextCursor& cursor);

  bool is_transmitting_call(QString const& line, QString const& callsign)
  {
      if (line.isEmpty() || callsign.isEmpty())
        return false;

      DecodedText decoded {line};
      return decoded.transmittingCall().toUpper() == callsign.toUpper();
  }
}

void DisplayText::highlight_callsign_line (QString const& callsign, QColor const& bg,
                                           QColor const& fg, bool last_period_only, bool bold)
{
  if (!callsign.size ())
    {
      return;
    }
  if (callsign == "CLEARALL!")
    {
      return;
    }
  auto regexp = callsign;
  QRegularExpression target {QString {"<?"}
                             + regexp.replace (QLatin1Char {'+'}, QLatin1String {"\\+"})
                                 .replace (QLatin1Char {'.'}, QLatin1String {"\\."})
                                 .replace (QLatin1Char {'?'}, QLatin1String {"\\?"})
                             + QString {">?"}
                             , QRegularExpression::DontCaptureOption};
  QTextCursor cursor {document ()};
  if (last_period_only)
    {
      cursor.movePosition (QTextCursor::End);
      QTextCursor period_start {cursor};
      QTextCursor prior {cursor};
      auto period_timestamp = get_timestamp (period_start);
      while (period_timestamp.size () && period_timestamp == get_timestamp (prior))
        {
          period_start = prior;
        }
      cursor = period_start;
    }
  while (!cursor.isNull ())
    {
      cursor = document ()->find (target, cursor, QTextDocument::FindWholeWords);
      if (!cursor.isNull () && cursor.hasSelection ())
        {
          cursor.select (QTextCursor::LineUnderCursor);
          auto format = cursor.charFormat ();
          if (bg.isValid ())
            format.setBackground (bg);
          else
            format.clearBackground ();
          if (fg.isValid ())
            format.setForeground (fg);
          else
            format.clearForeground ();
          bool lineBold = bold && is_transmitting_call(cursor.selectedText(), callsign);
          format.setFontWeight (lineBold ? QFont::Bold : QFont::Normal);
          cursor.mergeCharFormat (format);
        }
    }
}

namespace
{
  void update_selection (QTextCursor& cursor, QColor const& bg, QColor const& fg)
  {
    QTextCharFormat format {cursor.charFormat ()};
    if (bg.isValid ())
      {
        format.setBackground (bg);
      }
    else
      {
        format.clearBackground ();
      }
    if (fg.isValid ())
      {
        format.setForeground (fg);
      }
    else
      {
        format.clearForeground ();
      }
    cursor.mergeCharFormat (format);
  }

  void reset_selection (QTextCursor& cursor)
  {
    // restore previous text format, we rely on the text
    // char format at he start of the selection being the
    // old one which should be the case
    auto c2 = cursor;
    c2.setPosition (c2.selectionStart ());
    cursor.setCharFormat (c2.charFormat ());
  }
}

namespace
{
  QString get_timestamp (QTextCursor& cursor)
  {
    QString timestamp;
    if (cursor.movePosition (QTextCursor::PreviousCharacter)
        && cursor.movePosition (QTextCursor::StartOfLine)
        && cursor.movePosition (QTextCursor::EndOfWord, QTextCursor::KeepAnchor)
        && cursor.hasSelection ())
      {
        timestamp = cursor.selectedText ();
        cursor.movePosition (QTextCursor::StartOfLine);
      }
    return timestamp;
  }
}

void DisplayText::highlight_callsign (QString const& callsign, QColor const& bg,
                                      QColor const& fg, bool last_period_only)
{
  if (!callsign.size ())
    {
      return;
    }
  if (callsign == "CLEARALL!")  // programmatic means of clearing all highlighting
    {
      highlighted_calls_.clear();
      return;
    }
  auto regexp = callsign;
  // allow for hashed callsigns and escape any regexp metacharacters
  QRegularExpression target {QString {"<?"}
                             + regexp.replace (QLatin1Char {'+'}, QLatin1String {"\\+"})
                                 .replace (QLatin1Char {'.'}, QLatin1String {"\\."})
                                 .replace (QLatin1Char {'?'}, QLatin1String {"\\?"})
                             + QString {">?"}
                             , QRegularExpression::DontCaptureOption};
  QTextCharFormat old_format {currentCharFormat ()};
  QTextCursor cursor {document ()};
  if (last_period_only)
    {
      // highlight each instance of the given callsign (word) in the
      // current period
      cursor.movePosition (QTextCursor::End);
      QTextCursor period_start {cursor};
      QTextCursor prior {cursor};
      auto period_timestamp = get_timestamp (period_start);
      while (period_timestamp.size () && period_timestamp == get_timestamp (prior))
        {
          period_start = prior;
        }
      cursor = period_start;
      while (!cursor.isNull ())
        {
          cursor = document ()->find (target, cursor, QTextDocument::FindWholeWords);
          if (!cursor.isNull () && cursor.hasSelection ())
            {
              if (bg.isValid () || fg.isValid ())
                {
                  update_selection (cursor, bg, fg);
                }
              else
                {
                  reset_selection (cursor);
                }
            }
        }
    }
  else
    {
      auto pos = highlighted_calls_.find (callsign);
      if (bg.isValid () || fg.isValid ())
        {
          auto colours = qMakePair (bg, fg);
          if (pos == highlighted_calls_.end ())
            {
              pos = highlighted_calls_.insert (callsign.toUpper (), colours);
            }
          else
            {
              pos.value () = colours; // update colours
            }
          while (!cursor.isNull ())
            {
              cursor = document ()->find (target, cursor, QTextDocument::FindWholeWords);
              if (!cursor.isNull () && cursor.hasSelection ())
                {
                  update_selection (cursor, bg, fg);
                }
            }
        }
      else
        {
          if (pos != highlighted_calls_.end ())
            {
              highlighted_calls_.erase (pos);
            }
          QTextCursor cursor {document ()};
          while (!cursor.isNull ())
            {
              cursor = document ()->find (target, cursor, QTextDocument::FindWholeWords);
              if (!cursor.isNull () && cursor.hasSelection ())
                {
                  reset_selection (cursor);
                }
            }
        }
    }
  setCurrentCharFormat (old_format);
}

void DisplayText::flushUpdates() {
    for (const auto& update : m_updateBuffer) {
        QString text;
        QColor bg, fg;
        QString call1, call2;
        QTextCursor::MoveOperation location;
        bool bold;

        std::tie(text, bg, fg, call1, call2, location, bold) = update;

        insertText(text, bg, fg, call1, call2, location, bold);
    }
    m_updateBuffer.clear();
}

void DisplayText::enterEvent(QEvent *event) {
    if (m_config && m_config->freezeBA() && isBandActivity) {
        m_freezeUpdates = true;
        setStyleSheet("QTextEdit { border: 2px solid red; }"
                          "QScrollBar { border: none; }");
    }

    QTextEdit::enterEvent(event);
}

void DisplayText::leaveEvent(QEvent *event) {
    if (m_config && m_config->freezeBA() && isBandActivity) {
        m_freezeUpdates = false;
        setStyleSheet("");
        flushUpdates();
    }

    QTextEdit::leaveEvent(event);
}

void DisplayText::setBandActivity(bool b) {
    isBandActivity = b;
}
