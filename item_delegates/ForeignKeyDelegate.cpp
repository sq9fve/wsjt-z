#include "ForeignKeyDelegate.hpp"

#include <QApplication>
#include <QComboBox>
#include <QFontMetrics>
#include <QSize>
#include <QStyleOptionViewItem>
#include "CandidateKeyFilter.hpp"

ForeignKeyDelegate::ForeignKeyDelegate (QAbstractItemModel const * referenced_model
                                        , int referenced_key_column
                                        , QObject * parent
                                        , int referenced_key_role)
  : QStyledItemDelegate {parent}
  , candidate_key_filter_ {new CandidateKeyFilter {referenced_model, referenced_key_column, nullptr, referenced_key_role}}
{
  connect (candidate_key_filter_.data (), &QAbstractItemModel::dataChanged
           , this, [this] () {
             invalidate_size_hint_cache ();
           });
  connect (candidate_key_filter_.data (), &QAbstractItemModel::modelReset
           , this, [this] { invalidate_size_hint_cache (); });
  connect (candidate_key_filter_.data (), &QAbstractItemModel::rowsInserted
           , this, [this] () {
             invalidate_size_hint_cache ();
           });
  connect (candidate_key_filter_.data (), &QAbstractItemModel::rowsRemoved
           , this, [this] () {
             invalidate_size_hint_cache ();
           });
  connect (candidate_key_filter_.data (), &QAbstractItemModel::layoutChanged
           , this, [this] () {
             invalidate_size_hint_cache ();
           });
}

ForeignKeyDelegate::ForeignKeyDelegate (QAbstractItemModel const * referenced_model
                                        , QAbstractItemModel const * referencing_model
                                        , int referenced_key_column
                                        , int referencing_key_column
                                        , QObject * parent
                                        , int referenced_key_role
                                        , int referencing_key_role)
  : QStyledItemDelegate {parent}
  , candidate_key_filter_ {new CandidateKeyFilter {referenced_model, referencing_model, referenced_key_column, referencing_key_column, nullptr, referenced_key_role, referencing_key_role}}
{
  connect (candidate_key_filter_.data (), &QAbstractItemModel::dataChanged
           , this, [this] () {
             invalidate_size_hint_cache ();
           });
  connect (candidate_key_filter_.data (), &QAbstractItemModel::modelReset
           , this, [this] { invalidate_size_hint_cache (); });
  connect (candidate_key_filter_.data (), &QAbstractItemModel::rowsInserted
           , this, [this] () {
             invalidate_size_hint_cache ();
           });
  connect (candidate_key_filter_.data (), &QAbstractItemModel::rowsRemoved
           , this, [this] () {
             invalidate_size_hint_cache ();
           });
  connect (candidate_key_filter_.data (), &QAbstractItemModel::layoutChanged
           , this, [this] () {
             invalidate_size_hint_cache ();
           });
}

ForeignKeyDelegate::~ForeignKeyDelegate ()
{
}

void ForeignKeyDelegate::invalidate_size_hint_cache ()
{
  cached_size_hint_ = QSize {};
  cached_size_hint_font_ = QFont {};
  cached_size_hint_style_sheet_.clear ();
}

QWidget * ForeignKeyDelegate::createEditor (QWidget * parent
                                            , QStyleOptionViewItem const& /* option */
                                            , QModelIndex const& index) const
{
  auto editor = new QComboBox {parent};
  editor->setFrame (false);
  candidate_key_filter_->set_active_key (index);
  editor->setModel (candidate_key_filter_.data ());
  editor->setSizeAdjustPolicy (QComboBox::AdjustToContents);
  return editor;
}

QSize ForeignKeyDelegate::sizeHint (QStyleOptionViewItem const& option, QModelIndex const& index) const
{
  auto size_hint = QStyledItemDelegate::sizeHint (option, index);
  auto const style_sheet = qApp->styleSheet ();
  if (cached_size_hint_.isValid ()
      && cached_size_hint_font_ == option.font
      && cached_size_hint_style_sheet_ == style_sheet)
    {
      return size_hint.expandedTo (cached_size_hint_);
    }

  QFontMetrics metrics {option.font};
  QStyleOptionComboBox combo_box_option;
  combo_box_option.rect = option.rect;
  combo_box_option.state = option.state | QStyle::State_Enabled;
  cached_size_hint_ = QSize {};
  for (auto row = 0; row < candidate_key_filter_->rowCount (); ++row)
    {
      cached_size_hint_ = cached_size_hint_.expandedTo (
        qApp->style ()->sizeFromContents (
          QStyle::CT_ComboBox
          , &combo_box_option
          , metrics.boundingRect (candidate_key_filter_->data (
                                    candidate_key_filter_->index (row, 0)).toString ()).size ())
        );
    }
  cached_size_hint_font_ = option.font;
  cached_size_hint_style_sheet_ = style_sheet;
  return size_hint.expandedTo (cached_size_hint_);
}
