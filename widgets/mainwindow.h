// -*- Mode: C++ -*-
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QLabel>
#include <QThread>
#include <QProcess>
#include <QProgressBar>
#include <QTimer>
#include <QDateTime>
#include <QList>
#include <QAudioDeviceInfo>
#include <QScopedPointer>
#include <QDir>
#include <QProgressDialog>
#include <QAbstractSocket>
#include <QHostAddress>
#include <QPointer>
#include <QSet>
#include <QVector>
#include <QQueue>
#include <QFuture>
#include <QFutureWatcher>
#include <QDateTime>
// Z
#include <qrzlookup.h>

#include "MultiGeometryWidget.hpp"
#include "NonInheritingProcess.hpp"
#include "Audio/AudioDevice.hpp"
#include "commons.h"
#include "Radio.hpp"
#include "models/Modes.hpp"
#include "models/FrequencyList.hpp"
#include "Configuration.hpp"
#include "WSPR/WSPRBandHopping.hpp"
#include "Transceiver/Transceiver.hpp"
#include "DisplayManual.hpp"
#include "Network/PSKReporter.hpp"
#include "logbook/logbook.h"
#include "astro.h"
#include "MessageBox.hpp"
#include "Network/NetworkAccessManager.hpp"

#define NUM_JT4_SYMBOLS 206                //(72+31)*2, embedded sync
#define NUM_JT65_SYMBOLS 126               //63 data + 63 sync
#define NUM_JT9_SYMBOLS 85                 //69 data + 16 sync
#define NUM_WSPR_SYMBOLS 162               //(50+31)*2, embedded sync
#define NUM_MSK144_SYMBOLS 144             //s8 + d48 + s8 + d80
#define NUM_Q65_SYMBOLS 85                 //63 data + 22 sync
#define NUM_FT8_SYMBOLS 79
#define NUM_SUPERFOX_SYMBOLS 153         //24 sync + 127 data + 2 ramp up/down
#define NUM_FT4_SYMBOLS 105
#define NUM_FT2_SYMBOLS 105
#define NUM_FST4_SYMBOLS 160             //240/2 data + 5*8 sync
#define NUM_CW_SYMBOLS 250
#define MAX_NUM_SYMBOLS 250
#define TX_SAMPLE_RATE 48000
#define NRING 3456000
#define MAX_HOUNDS_IN_QUEUE 10

extern int volatile itone[MAX_NUM_SYMBOLS];   //Audio tones for all Tx symbols
extern int volatile icw[NUM_CW_SYMBOLS];	    //Dits for CW ID

//--------------------------------------------------------------- MainWindow
namespace Ui {
  class MainWindow;
}

class QProcessEnvironment;
class QSharedMemory;
class QSplashScreen;
class QSettings;
class QLineEdit;
class QPoint;
class QFont;
class QHostInfo;
class EchoGraph;
class FastGraph;
class WideGraph;
class LogQSO;
class Transceiver;
class MessageAveraging;
class ActiveStations;
class FoxLogWindow;
class CabrilloLogWindow;
class QSYMessageCreator;
class QSYMessage;
class QSYMonitor;
class ColorHighlighting;
class MessageClient;
class QTime;
class WSPRBandHopping;

// Z
class UnfilteredView;
class PSKReporterWidget;

class HelpTextWindow;
class WSPRNet;
class SoundOutput;
class Modulator;
class SoundInput;
class Detector;
class SampleDownloader;
class MultiSettings;
class EqualizationToolsDialog;
class DecodedText;

class MainWindow
  : public MultiGeometryWidget<3, QMainWindow>
{
  Q_OBJECT;

public:
  using Frequency = Radio::Frequency;
  using FrequencyDelta = Radio::FrequencyDelta;
  using Mode = Modes::Mode;
  using SpecOp = Configuration::SpecialOperatingActivity;

  explicit MainWindow(QDir const& temp_directory, bool multiple, MultiSettings *,
                      QSharedMemory *shdmem, unsigned downSampleFactor,
                      QSplashScreen *, QProcessEnvironment const&,
                      QWidget *parent = nullptr);
  ~MainWindow();

  int decoderBusy () const {return m_decoderBusy;}
  // Z
  void log(QString s);

public slots:
  void showSoundInError(const QString& errorMsg);
  void showSoundOutError(const QString& errorMsg);
  void showStatusMessage(const QString& statusMsg);
  void dataSink(qint64 frames);
  void fastSink(qint64 frames);
  void tci_mod_active(bool on) {m_tci_mod_active = on;}
  void diskDat();
  void freezeDecode(int n);
  void guiUpdate();
  void doubleClickOnCall (Qt::KeyboardModifiers);
  void doubleClickOnCall2(Qt::KeyboardModifiers);
  void doubleClickOnFoxQueue(Qt::KeyboardModifiers);
  void doubleClickOnFoxInProgress(Qt::KeyboardModifiers modifiers);
  void readFromStdout();
  void p1ReadFromStdout();
  void setXIT(int n, Frequency base = 0u);
  void setFreq4(int rxFreq, int txFreq);
  void msgAvgDecode2();
  void fastPick(int x0, int x1, int y);

private:
  void change_layout (std::size_t) override;
  void keyPressEvent (QKeyEvent *) override;
  void closeEvent(QCloseEvent *) override;
  void childEvent(QChildEvent *) override;
  bool eventFilter(QObject *, QEvent *) override;

private slots:
  void initialize_fonts ();
  void stopWRTimeout();
  void on_houndButton_clicked(bool checked);
  void on_ft8Button_clicked();
  void on_ft4Button_clicked();
  void on_msk144Button_clicked();
  void on_q65Button_clicked();
  void on_jt65Button_clicked();
  void on_tx1_editingFinished();
  void on_tx2_editingFinished();
  void on_tx3_editingFinished();
  void on_tx4_editingFinished();
  void on_tx5_currentTextChanged (QString const&);
  void on_tx6_editingFinished();
  void on_actionSettings_triggered();
  void on_monitorButton_clicked (bool);
  void on_actionAbout_triggered();
  void on_autoButton_clicked (bool);
  void on_stopTxButton_clicked();
  void on_stopButton_clicked();
  void on_actionRelease_Notes_triggered ();
  void on_actionFT8_DXpedition_Mode_User_Guide_triggered();
  void on_actionSuperFox_User_Guide_triggered();
  void on_actionQSG_FST4_triggered();
  void on_actionQSG_Q65_triggered();
  void on_actionQSG_X250_M3_triggered();
  void on_actionQuick_Start_Guide_to_WSJT_X_2_7_and_QMAP_triggered();
  void on_actionOnline_User_Guide_triggered();
  void on_actionLocal_User_Guide_triggered();
  void on_actionWide_Waterfall_triggered();
  void on_actionOpen_triggered();
  void on_actionOpen_next_in_directory_triggered();
  void on_actionDecode_remaining_files_in_directory_triggered();
  void on_actionDelete_all_wav_files_in_SaveDir_triggered();
  void on_actionOpen_log_directory_triggered ();
  void on_actionNone_triggered();
  void on_actionSave_all_triggered();
  void on_actionKeyboard_shortcuts_triggered();
  void on_actionSpecial_mouse_commands_triggered();
  void on_actionSolve_FreqCal_triggered();
  void on_actionCopyright_Notice_triggered();
  void on_actionSWL_Mode_triggered (bool checked);
  void on_DecodeButton_clicked (bool);
  void decode();
  void decodeBusy(bool b);
  void on_EraseButton_clicked();
  void band_activity_cleared ();
  void rx_frequency_activity_cleared ();
  void on_txFirstCheckBox_stateChanged(int arg1);
  void set_dateTimeQSO(int m_ntx);
  void set_ntx(int n);
  void on_txrb1_toggled(bool status);
  void on_txrb1_doubleClicked ();
  void on_txrb2_toggled(bool status);
  void on_txrb3_toggled(bool status);
  void on_txrb4_toggled(bool status);
  void on_txrb4_doubleClicked ();
  void on_txrb5_toggled(bool status);
  void on_txrb5_doubleClicked ();
  void on_txrb6_toggled(bool status);
  void on_txb1_clicked();
  void on_txb1_doubleClicked ();
  void on_txb2_clicked();
  void on_txb3_clicked();
  void on_txb4_clicked();
  void on_txb4_doubleClicked ();
  void on_txb5_clicked();
  void on_txb5_doubleClicked ();
  void on_txb6_clicked();
  void on_lookupButton_clicked();
  void on_addButton_clicked();
  void wheelEvent(QWheelEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void on_dxCallEntry_textChanged (QString const&);
  void on_dxGridEntry_textChanged (QString const&);
  void on_dxCallEntry_editingFinished();
  void on_dxCallEntry_returnPressed ();
  void on_genStdMsgsPushButton_clicked();
  void on_logQSOButton_clicked();
  void on_actionJT9_triggered();
  void on_actionJT65_triggered();
  void on_actionJT4_triggered();
  void on_actionFT4_triggered();
  void on_actionFT2_triggered();
  void on_actionFT8_triggered();
  void on_actionFST4_triggered();
  void on_actionFST4W_triggered();
  void on_TxFreqSpinBox_valueChanged(int arg1);
  void on_actionSave_decoded_triggered();
  void on_actionQuickDecode_toggled (bool);
  void on_actionMediumDecode_toggled (bool);
  void on_actionDeepestDecode_toggled (bool);
  void bumpFqso(int n);
  void on_actionErase_ALL_TXT_triggered();
  void on_reset_cabrillo_log_action_triggered ();
  void on_actionErase_wsjtx_log_adi_triggered();
  void on_actionErase_WSPR_hashtable_triggered();
  void on_actionErase_list_of_Q65_callers_triggered();
  void on_actionExport_Cabrillo_log_triggered();
  void startTx2();
  void startP1();
  void stopTx();
  void stopTx2();
  void on_rptSpinBox_valueChanged(int n);
  void killFile();
  void on_tuneButton_clicked (bool);
  void on_pbR2T_clicked();
  void on_pbT2R_clicked();
  void acceptQSO (QDateTime const&, QString const& call, QString const& grid
                  , Frequency dial_freq, QString const& mode
                  , QString const& rpt_sent, QString const& rpt_received
                  , QString const& tx_power, QString const& comments
                  , QString const& name, QDateTime const& QSO_date_on, QString const& operator_call
                  , QString const& my_call, QString const& my_grid
                  , QString const& exchange_sent, QString const& exchange_rcvd
                  , QString const& propmode, QByteArray const& ADIF);
  void on_bandComboBox_currentIndexChanged (int index);
  void on_bandComboBox_editTextChanged (QString const& text);
  void on_bandComboBox_activated (int index);
  void on_readFreq_clicked();
  void on_RxFreqSpinBox_valueChanged(int n);
  void on_outAttenuation_valueChanged (int);
  void rigOpen ();
  void handle_transceiver_update (Transceiver::TransceiverState const&);
  void handle_transceiver_failure (QString const& reason);
  void on_actionAstronomical_data_toggled (bool);
  void on_actionQSYMessage_Creator_triggered();
  void on_actionQSY_Monitor_triggered();
  void on_actionShort_list_of_add_on_prefixes_and_suffixes_triggered();
  void band_changed (Frequency);
  void monitor (bool);
  void end_tuning ();
  void stop_tuning ();
  void stopTuneATU();
  void auto_tx_mode(bool);
  void on_actionMessage_averaging_triggered();
  void on_actionActiveStations_triggered();
  void on_contest_log_action_triggered ();
  void on_fox_log_action_triggered ();
  void on_actionColors_triggered();
  void on_actionInclude_averaging_toggled (bool);
  void on_actionInclude_correlation_toggled (bool);
  void on_actionEnable_AP_DXcall_toggled (bool);
  void on_actionAuto_Clear_Avg_toggled (bool);
  void VHF_features_enabled(bool b);
  void on_sbSubmode_valueChanged(int n);
  void on_cbSendMsg_toggled(bool b);
  void on_cbShMsgs_toggled(bool b);
  void on_cbSWL_toggled(bool b);
  void on_cbTx6_toggled(bool b);
  void on_cbMenus_toggled(bool b);
  void on_cbCQonly_toggled(bool b);
  void on_cbAutoSeq_toggled(bool b);
  void on_cbFirst_toggled(bool checked);
  void on_respondComboBox_currentIndexChanged(int index);
  void networkError (QString const&);
  void on_ClrAvgButton_clicked();
  void on_actionWSPR_triggered();
  void on_syncSpinBox_valueChanged(int n);
  void on_TxPowerComboBox_currentIndexChanged(int);
  void on_sbTxPercent_valueChanged(int n);
  void on_cbUploadWSPR_Spots_toggled(bool b);
  void WSPR_config(bool b);
  void uploadWSPRSpots (bool direct_post = false, QString const& decode_text = QString {});
  void TxAgain();
  void uploadResponse(QString const& response);
  void on_WSPRfreqSpinBox_valueChanged(int n);
  void on_sbFST4W_RxFreq_valueChanged(int n);
  void on_sbFST4W_FTol_valueChanged(int n);
  void on_pbTxNext_clicked(bool b);
  void on_actionEcho_Graph_triggered();
  void on_actionEcho_triggered();
  void on_actionFast_Graph_triggered();
  void fast_decode_done();
  void on_actionMeasure_reference_spectrum_triggered();
  void on_actionErase_reference_spectrum_triggered();
  void on_actionMeasure_phase_response_triggered();
  void on_sbTR_valueChanged (int);
  void on_sbTR_FST4W_valueChanged (int);
  void on_sbFtol_valueChanged (int);
  void on_cbFast9_clicked(bool b);
  void on_sbCQTxFreq_valueChanged(int n);
  void on_cbCQTx_toggled(bool b);
  void on_actionMSK144_triggered();
  void on_actionQ65_triggered();
  void on_actionFreqCal_triggered();
  void splash_done ();
  void on_measure_check_box_stateChanged (int);
  void on_sbNlist_valueChanged(int n);
  void on_sbNslots_valueChanged(int n);
  void on_sbF_Low_valueChanged(int n);
  void on_sbF_High_valueChanged(int n);
  void chk_FST4_freq_range();
  void on_pbFoxReset_clicked();
  void on_pbFreeText_clicked();
  void FoxReset(QString reason);
  void on_comboBoxHoundSort_activated (int index);
  void not_GA_warning_message ();
  void checkMSK144ContestType();
  void on_pbBestSP_clicked();
  void on_RoundRobin_currentTextChanged(QString text);
  void setTxMsg(int n);
  bool stdCall(QString const& w);
  void remote_configure (QString const& mode, quint32 frequency_tolerance, QString const& submode
                         , bool fast_mode, quint32 tr_period, quint32 rx_df, QString const& dx_call
                         , QString const& dx_grid, bool generate_messages);
  void callSandP2(int nline);
  void refreshHoundQueueDisplay();
  void queueActiveWindowHound2(QString text);
	// Z
    void pskTableClicked(QString,  QString);
    void logSlots();
    void execCmd(QString cmd);
    bool setFreeFreq();
    bool isSlotFree(int f);
    void addSlot(int freq);
    void on_pb_FreeFreq_clicked();
     void on_cbAutoCall_toggled(bool b);
     void on_cbAutoCQ_toggled(bool b);
     bool callsignFiltered(DecodedText dt);
     // FT8 thread count selectors (Decode > Parameters > Number of threads)
     void on_actionMTAuto_triggered();
     void on_actionMT1_triggered();
     void on_actionMT2_triggered();
     void on_actionMT3_triggered();
     void on_actionMT4_triggered();
     void on_actionMT5_triggered();
     void on_actionMT6_triggered();
     void on_actionMT7_triggered();
     void on_actionMT8_triggered();
     void on_actionMT9_triggered();
     void on_actionMT10_triggered();
     void on_actionMT11_triggered();
     void on_actionMT12_triggered();
     void on_actionUse_multithreaded_FT8_decoder_toggled(bool b);
     // Decode > Parameters > Decoder sensitivity (FT8): radio group, value 1/2/3
     void on_actionFT8SensMin_toggled(bool checked);
     void on_actionlowFT8thresholds_toggled(bool checked);
     void on_actionFT8subpass_toggled(bool checked);
     // Decode > Parameters > Decoder start (FT8): radio group, value 0..4
     void on_actionStartTwoStage_toggled(bool checked);
     void on_actionStartThreeStage_toggled(bool checked);
     void on_actionStartEarly_toggled(bool checked);
     void on_actionStartNormal_toggled(bool checked);
     void on_actionStartLate_toggled(bool checked);
     // Decode > Parameters > QSO RX freq sensitivity: radio group, value 1/2/3
     void on_actionRXfLow_triggered();
     void on_actionRXfMedium_triggered();
     void on_actionRXfHigh_triggered();
     // Decode > Parameters > Decoding cycles: radio group, value 1/2/3
     void on_actionDecFT8cycles1_triggered();
     void on_actionDecFT8cycles2_triggered();
     void on_actionDecFT8cycles3_triggered();
     // Decode > Wideband DX Call search
     void on_actionFT8WidebandDXCallSearch_toggled(bool checked);
     void on_btn_addToIgnore_clicked();
     void on_btn_clearIgnore_clicked();
     void on_actionIgnore_station_triggered();
     void on_actionCall_next_triggered();
     void on_actionClear_triggered();
     void on_cb_autoCallNext_toggled(bool b);
     void on_cbMini_toggled(bool b);
     void on_cbCQonlyIncl73_toggled(bool b);
     void on_cb_filtering_toggled (bool b);
     void on_actionSet_Rx_Freq_triggered();
     void on_actionQRZ_Lookup_triggered();
     void on_actionCopy_triggered();
     void moveEvent(QMoveEvent *) override;
     void hideEvent(QHideEvent *) override;
     void showEvent(QShowEvent *) override;
     void changeEvent(QEvent *) override;
     void on_txrb6_doubleClicked ();
     void dxLookup(QString dxCall, QString dxGrid);
     void leftClickHandler(Qt::KeyboardModifiers modifiers);
    void bandActivityClickToggle(Qt::KeyboardModifiers modifiers);
     void on_actionCall_info_triggered();
     void on_actionDark_mode_triggered();
     void qrzInit();
     void qrzSetSessionKey(QNetworkReply *r) ;
     void qrzResponseHandler(QNetworkReply * r);
     void on_q_button_clicked();
     void on_ci_pb_lookup_clicked();
     void on_cb_specialMode_currentIndexChanged( int index);
     void on_cb_autoModeSwitch_toggled(bool b);
    void on_cb_autoModeSwitch_customContextMenuRequested(QPoint const&);
     void on_actionAbout_WSJT_Z_triggered();
     void on_pb_WDReset_clicked();
     void resetAutoSwitch();
    double watchdog();
     void on_actionUnfiltered_View_triggered();
     void on_actionPSKReporter_triggered();
     void updateQsoCounter(bool increment);
     void on_txFirstCheckBox_toggled();
    void update_tx5(const QString &qsy_text);
    void reply_tx5(const QString &qsy_reply);

private:
  Q_SIGNAL void initializeAudioOutputStream (QAudioDeviceInfo,
      unsigned channels, unsigned msBuffered) const;
  Q_SIGNAL void stopAudioOutputStream () const;
  Q_SIGNAL void startAudioInputStream (QAudioDeviceInfo const&,
      int framesPerBuffer, AudioDevice * sink,
      unsigned downSampleFactor, AudioDevice::Channel) const;
  Q_SIGNAL void suspendAudioInputStream () const;
  Q_SIGNAL void resumeAudioInputStream () const;
  Q_SIGNAL void startDetector (AudioDevice::Channel) const;
  Q_SIGNAL void FFTSize (unsigned) const;
  Q_SIGNAL void detectorClose () const;
  Q_SIGNAL void finished () const;
  Q_SIGNAL void transmitFrequency (double) const;
  Q_SIGNAL void endTransmitMessage (bool quick = false) const;
  Q_SIGNAL void tune (bool = true) const;
  Q_SIGNAL void sendMessage (QString mode, unsigned symbolsLength,
      double framesPerSymbol, double frequency, double toneSpacing,
      SoundOutput *, AudioDevice::Channel = AudioDevice::Mono,
      bool synchronize = true, bool fastMode = false, double dBSNR = 99.,
                             int TRperiod=60) const;
  Q_SIGNAL void outAttenuationChanged (qreal) const;
  Q_SIGNAL void toggleShorthand () const;
  Q_SIGNAL void reset_audio_input_stream (bool report_dropped_frames) const;

private:
  void set_mode (QString const& mode);
  void astroUpdate ();
  void writeAllTxt(QString message);
  void auto_sequence (DecodedText const& message, unsigned start_tolerance, unsigned stop_tolerance);
  void trim_view (bool b);
  void foxTest();
  void setColorHighlighting();
  void chkFT4();
  bool elide_tx1_not_allowed () const;
  void readWidebandDecodes();
  void configActiveStations();
  void showQSYMessage(QString message);
  void sfox_tx();
  // Z
  void ci_gridLookup();
  void toggleBands();
  void switchBand(int row);
  void ZMessage();
  void ZProcess();
  void clearPounceState();

  // Filter cache: parsed once when the QPlainTextEdit changes, reused per-decode.
  // Invalidated by textChanged signals connected in the ctor.
  void invalidateFilterCache() { m_filterCacheValid = false; }
  void rebuildFilterCache() const;
  mutable bool m_filterCacheValid = false;
  mutable QStringList m_ignoredStationsCache;
  mutable QStringList m_prefixFilterLinesCache;
  mutable QStringList m_stateFilterLinesCache;

  // Parallel FT8 decoder thread count (0 = OpenMP auto-detect; 1..12 = pinned).
  // Bound to the Decode > Parameters > Number of threads radio menu.
  int m_ft8threads = 0;

  // FT8 decoder sensitivity (Decode > Parameters > Decoder sensitivity).
  // 1 = Min (lft8lowth=false), 2 = low thresholds, 3 = subpass (default).
  qint32 m_ft8Sensitivity = 3;
  // FT8 decoder start time (Decode > Parameters > Decoder start).
  // 0 = 2-Stage, 1 = 3-Stage, 2 = Early, 3 = Normal (default), 4 = Late.
  qint32 m_ft8DecoderStart = 3;
  // QSO RX frequency sensitivity (Decode > Parameters > QSO RX freq sensitivity).
  // 1 = Low, 2 = Medium (default), 3 = High.
  qint32 m_nFT8RXfSens = 2;
  // FT8 decoding cycles (Decode > Parameters > Decoding cycles). 1, 2, or 3 (default).
  qint32 m_nFT8Cycles = 3;
  // Wideband DX call search (Decode > Wideband DX Call search). Drives dec_data.params.lwidedxcsearch.
  bool m_FT8WideDxCallSearch = false;

  // Cached state of actionWSJT_Z_Debug to avoid widget lookup on every log() call.
  // Updated by the action's toggled signal in the ctor. Hot-path log() sites should
  // gate on this directly: `if (m_zdebug) log("…" + foo)` to skip QString construction
  // when debug is off.
  bool m_zdebug = false;

  QProcessEnvironment const& m_env;
  NetworkAccessManager m_network_manager;
  bool m_valid;
  QSplashScreen * m_splash;
  QString m_revision;
  bool m_multiple;
  MultiSettings * m_multi_settings;
  QPushButton * m_configurations_button;
  QSettings * m_settings;
  QScopedPointer<Ui::MainWindow> ui;

  Configuration m_config;
  LogBook m_logBook;            // must be after Configuration construction
  WSPRBandHopping m_WSPR_band_hopping;
  bool m_WSPR_tx_next;
  MessageBox m_rigErrorMessageBox;
  QScopedPointer<SampleDownloader> m_sampleDownloader;
  QScopedPointer<EqualizationToolsDialog> m_equalizationToolsDialog;

  QScopedPointer<WideGraph> m_wideGraph;
  QScopedPointer<EchoGraph> m_echoGraph;
  QScopedPointer<FastGraph> m_fastGraph;
  QScopedPointer<LogQSO> m_logDlg;
  QScopedPointer<Astro> m_astroWidget;
  QScopedPointer<QSYMessageCreator> m_QSYMessageCreatorWidget;
  QScopedPointer<QSYMessage> m_QSYMessageWidget;
  QScopedPointer<QSYMonitor> m_qsymonitorWidget;
  QScopedPointer<HelpTextWindow> m_shortcuts;
  QScopedPointer<HelpTextWindow> m_prefixes;
  QScopedPointer<HelpTextWindow> m_mouseCmnds;
  QScopedPointer<MessageAveraging> m_msgAvgWidget;
  QScopedPointer<ActiveStations> m_ActiveStationsWidget;
  QScopedPointer<FoxLogWindow> m_foxLogWindow;
  QScopedPointer<CabrilloLogWindow> m_contestLogWindow;
  QScopedPointer<ColorHighlighting> m_colorHighlighting;
  Transceiver::TransceiverState m_rigState;
  Frequency  m_lastDialFreq;
  QString m_lastBand;
  QString m_lastCallsign;
  Frequency  m_dialFreqRxWSPR;  // best guess at WSPR QRG

  Detector * m_detector;
  unsigned m_FFTSize;
  SoundInput * m_soundInput;
  Modulator * m_modulator;
  SoundOutput * m_soundOutput;
  int m_rx_audio_buffer_frames;
  int m_tx_audio_buffer_frames;
  QThread m_audioThread;

  qint64  m_msErase;
  int     m_nEraseClicks;
  qint64  m_secBandChanged;
  qint64  m_freqMoon;
  qint64  m_fullFoxCallTime;

  Frequency m_freqNominal;
  Frequency m_freqNominalPeriod;
  Frequency m_freqTxNominal;
  Astro::Correction m_astroCorrection;
  bool m_reverse_Doppler;

  double  m_tRemaining;
  double  m_TRperiod;
  double  m_fSpread;
  double  m_s6;
  double  m_fDither;
  double  m_fAudioShift;

  float   m_DTtol;
  float   m_t0;
  float   m_t1;
  float   m_t0Pick;
  float   m_t1Pick;
  float   m_fCPUmskrtd;

  qint32  m_waterfallAvg;
  qint32  m_ntx;
  bool m_gen_message_is_cq;
  bool m_send_RR73;
  qint32  m_timeout;
  qint32  m_XIT;
  qint32  m_setftx;
  qint32  m_ndepth;
  qint32  m_sec0;
  qint32  m_RxLog;
  qint32  m_nutc0;
  qint32  m_ntr;
  qint32  m_tx;
  qint32  m_hsym;
  qint32  m_nsps;
  qint32  m_hsymStop;
  qint32  m_inGain;
  qint32  m_ncw;
  qint32  m_secID;
  double  m_idleMinutes;
  qint32  m_nSubMode;
  qint32  m_nSubMode_Q65;
  qint32  m_nSubMode_JT65;
  qint32  m_nSubMode_JT4;
  qint32  m_nclearave;
  qint32  m_minSync;
  qint32  m_dBm;
  qint32  m_nWSPRdecodes;
  qint32  m_k0;
  qint32  m_kdone;
  qint32  m_nPick;
  FrequencyList_v2_101::const_iterator m_frequency_list_fcal_iter;
  qint32  m_nTx73;
  qint32  m_UTCdisk;
  QDateTime    m_UTCdiskDateTime;
  qint32  m_wait;
  qint32  m_isort;
  qint32  m_max_dB;
  qint32  m_nDXped=0;
  qint32  m_nSortedHounds=0;
  qint32  m_nHoundsCalling=0;
  qint32  m_Nlist=12;
  qint32  m_Nslots=5;
  qint32  m_Nslots0=0;
  qint32  m_nFoxMsgTimes[5]={0,0,0,0,0};
  qint32  m_tAutoOn;
  qint32  m_tFoxTx=0;
  qint32  m_tFoxTx0=0;
  qint32  m_maxStrikes=3;      //Max # of repeats: 3 strikes and you're out
  qint32  m_maxFoxWait=3;      //Max wait time for expected Hound replies
  qint32  m_foxCQtime=10;      //CQs at least every 5 minutes
  qint32  m_tFoxTxSinceCQ=999; //Fox Tx cycles since most recent CQ
  qint32  m_nFoxFreq;          //Audio freq at which Hound received a call from Fox
  qint32  m_nSentFoxRrpt=0;    //Serial number for next R+rpt Hound will send to Fox
  qint32  m_kin0=0;
  qint32  m_earlyDecode=41;
  qint32  m_earlyDecode2=47;
  qint32  m_nDecodes=0;
  qint32  m_maxPoints=-1;
  qint32  m_latestDecodeTime=-1;
  qint32  m_points=-99;
  qint32  m_score=0;
  qint32  m_fDop=0;
  qint32  m_echoSec0=0;
  qint32  m_fetched=0;

  bool    m_btxok;		//True if OK to transmit
  bool    m_tci_mod_active;
  bool    m_tci;
  bool    m_tci_audio;
  bool    inSettings = false;
  bool    m_diskData;
  bool    m_loopall;
  bool    m_decoderBusy;
  bool    m_txFirst;
  bool    m_auto;
  bool    m_restart;
  bool    m_startAnother;
  bool    m_saveDecoded;
  bool    m_saveAll;
  bool    m_widebandDecode;
  bool    m_call3Modified;
  bool    m_dataAvailable;
  bool    m_bDecoded;
  bool    m_noSuffix;
  bool    m_decodedText2;
  bool    m_sentFirst73;
  int     m_currentMessageType;
  QString m_currentMessage;
  int     m_lastMessageType;
  QString m_lastMessageSent;
  QString m_tBlankLine;
  bool    m_bShMsgs;
  bool    m_bSWL;
  bool    m_uploadWSPRSpots;
  bool    m_uploading;
  bool    m_grid6;
  bool    m_tuneup;
  bool    m_bTxTime;
  bool    m_rxDone;
  bool    m_bSimplex; // not using split even if it is available
  bool    m_bEchoTxOK;
  bool    m_bTransmittedEcho;
  bool    m_bEchoTxed;
  bool    m_bFastMode;
  bool    m_bFast9;
  bool    m_bFastDecodeCalled;
  bool    m_bDoubleClickAfterCQnnn;
  bool    m_bRefSpec;
  bool    m_bClearRefSpec;
  bool    m_bTrain;
  bool    m_bUseRef;
  bool    m_bFastDone;
  bool    m_bAltV;
  bool    m_bNoMoreFiles;
  bool    m_bDoubleClicked;
  bool    m_bCallingCQ;
  bool    m_bAutoReply;
  bool    m_bCheckedContest;
  bool    m_bWarnedSplit=false;
  bool    m_bTUmsg;
  bool    m_bBestSPArmed=false;
  bool    m_bOK_to_chk=false;
  bool    m_bSentReport=false;
  bool    m_discard_decoded_hounds_this_cycle=false;     // if something changes, like frequency, discard decoded messages that may be in-flight.

  SpecOp  m_specOp;
  // Z
  bool    m_autoTXFreq=false;
  bool    m_beeped=false;
  QString m_lastCall;
  int	  m_lastX = -1;
  int	  m_lastY = -1;
  int     m_maxDistance = 0;
  int     m_maxSignal = -30;
  QString m_priorityCall;
  bool    m_priorityCallPreferCQTarget = false;
  int     m_decodeCount = 0;
  bool    m_prioTxFirst;
  int     m_prioFreq;
  QString m_prioGrid;
  bool    m_infoMessageShown = false;
  bool    m_autoModeSwitch = false;
  bool    m_smartModeSwitch = false;
  bool    m_autoCQAlternateEvenOddNext = false;
  QScopedPointer<UnfilteredView> m_unfilteredView;
  QScopedPointer<PSKReporterWidget> m_pskReporterView;
  QThread * m_pskReporterThread;
  QDateTime m_ignoreListReset;
  QDateTime m_watchdogAnchorUtc;
  qint64 m_msTxFirst;
  bool m_TxFirstLock = false;
  bool m_AutoTxFreq = false;
  int qso_total = 0;
  int qso_new = 0;
  QByteArray m_unfilteredViewGeometry;
  QByteArray m_pskReporterViewGeometry;
  
  enum
    {
      CALLING,
      REPLYING,
      REPORT,
      ROGER_REPORT,
      ROGERS,
      SIGNOFF
    }
    m_QSOProgress;        //State machine counter

  enum {CALL, GRID, DXCC, MULT};

  int			m_ihsym;
  int			m_nzap;
  int			m_npts8;
  float		m_px;
  float   m_pxmax;
  float		m_df3;
  int			m_iptt0;
  bool		m_btxok0;
  int			m_nsendingsh;
  double	m_onAirFreq0;
  double	m_foxWarningDialFreq0;
  bool		m_first_error;

  char    m_msg[100][80];

  // labels in status bar
  QLabel tx_status_label;
  QLabel config_label;
  QLabel mode_label;
  QLabel last_tx_label;
  // Z
  QVector<QVector<int>> busySlots;
  QLabel qso_count;
  QLabel labAz;
  QLabel labDate;
  QString qrzSessionKey;
  QString qrzPendingLookupCall;
  QNetworkAccessManager * networkManager;

  QLabel auto_tx_label;
  QLabel band_hopping_label;
  QLabel ndecodes_label;
  QLabel mode_switch_status_label;
  QProgressBar progressBar;
  QLabel watchdog_label;

  QFuture<void> m_wav_future;
  QFutureWatcher<void> m_wav_future_watcher;
  QFutureWatcher<void> watcher3;
  QFutureWatcher<QString> m_saveWAVWatcher;

  NonInheritingProcess proc_jt9;
  NonInheritingProcess p1;
  NonInheritingProcess p3;
  QByteArray m_jt9_stderr_buffer;

  QProcess p2;
  QProcess p4;

  WSPRNet *wsprNet;

  QTimer m_guiTimer;
  QTimer stopWRTimer;
  QTimer ptt1Timer;                 //StartTx delay
  QTimer ptt0Timer;                 //StopTx delay
  QTimer logQSOTimer;
  QTimer killFileTimer;
  QTimer tuneButtonTimer;
  QTimer uploadTimer;
  QTimer tuneATU_Timer;
  QTimer TxAgainTimer;
  QTimer minuteTimer;
  QTimer splashTimer;
  QTimer p1Timer;

  QString m_path;
  QString m_baseCall;
  QString m_hisCall;
  QString m_hisGrid;
  QString m_appDir;
  QString m_cqStr;
  QString m_palette;
  QString m_dateTime;
  QString m_mode;
  QString m_fnameWE;            // save path without extension
  QString m_rpt;
  QString m_nextRpt;
  QString m_rptSent;
  QString m_rptRcvd;
  QString m_qsoStart;
  QString m_qsoStop;
  QStringList m_cmndP1;
  QString m_msgSent0;
  QString m_calls;
  QString m_CQtype;
  QString m_opCall;
  QString m_houndCallers;        //Sorted list of Hound callers
  QString m_fm0;
  QString m_fm1;
  QString m_xSent;               //Contest exchange sent
  QString m_xRcvd;               //Contest exchange received
  QString m_currentBand;
  QString m_currentBandPeriod;
  QString m_nextCall;
  QString m_nextGrid;
  QString m_fileDateTime;
  QString m_inQSOwith;
  QString m_BestCQpriority;
  QString m_deCall;
  QString m_deGrid;
  QString m_freeTextMsg;
  QString m_freeTextMsg0;
  QString m_ready2call[50];
  QString m_callers[50];

  QSet<QString> m_pfx;
  QSet<QString> m_sfx;

  struct FoxQSO       //Everything we need to know about QSOs in progress (or recently logged).
  {
    QString grid;       //Hound's declared locator
    QString sent;       //Report sent to Hound
    QString rcvd;       //Report received from Hound
    qint32  ncall;      //Number of times report sent to Hound
    qint32  nRR73;      //Number of times RR73 sent to Hound
    qint32  tFoxRrpt;   //m_tFoxTx (Fox Tx cycle counter) when R+rpt was received from Hound
    qint32  tFoxTxRR73; //m_tFoxTx when RR73 was sent to Hound
  };

  QMap<QString,FoxQSO> m_foxQSO;       //Key = HoundCall, value = parameters for QSO in progress
  QMap<QString,QString> m_loggedByFox; //Key = HoundCall, value = logged band
  QMap<QString,qint32> m_annotated_callsigns;  //Key = HoundCall, value = provided by api call

  struct FixupQSO       //Info for fixing Fox's log from file "FoxQSO.txt"
  {
    QString grid;       //Hound's declared locator
    QString sent;       //Report sent to Hound
    QString rcvd;       //Report received from Hound
    QDateTime QSO_time;
  };
  QMap<QString,FixupQSO> m_fixupQSO;       //Key = HoundCall, value = info for QSO in progress

  struct ActiveCall
  {
    QString grid4;
    QString bands;
    qint32 az;
    qint32 points;
  };
  QMap<QString,ActiveCall> m_activeCall;   //Key = callsign, value = grid4, az, points for ARRL_DIGI

  struct EMECall
  {
    QString grid4;
    double frx;
    double fsked;
    qint32 nsnr;
    qint32 t;
    bool worked;
    bool ready2call;
    QString submode;
  };
  QMap<QString,EMECall> m_EMECall;

  QMap<QString,bool> m_EMEworked;

  struct RecentCall
  {
    qint64 dialFreq;
    qint32 audioFreq;
    qint32 snr;
    qint32 decodeTime;
    bool   txEven;
    bool   ready2call;
  };
  QMap<QString,RecentCall> m_recentCall;   //Key = callsign, value = snr, dialFreq, audioFreq, decodeTime

  struct ARRL_logged
  {
    QDateTime time;
    QString band;
    qint32 points;
  };
  QList<ARRL_logged> m_arrl_log;

  QQueue<QString> m_houndQueue;        //Selected Hounds available for starting a QSO
  QQueue<QString> m_foxQSOinProgress;  //QSOs in progress: Fox has sent a report

  QDateTime m_dateTimeQSOOn;
  QDateTime m_dateTimeLastTX;
  QDateTime m_dateTimeSentTx3;
  QDateTime m_dateTimeRcvdRR73;
  QDateTime m_dateTimeBestSP;
  QDateTime m_dateTimeSeqStart;        //Nominal start time of Rx sequence about to be decoded

  QSharedMemory *mem_jt9;
  QString m_QSOText;
  unsigned m_downSampleFactor;
  QThread::Priority m_audioThreadPriority;
  bool m_bandEdited;
  bool m_splitMode;
  bool m_monitoring=false;
  bool m_echoRunning=false;
  bool m_tx_when_ready;
  bool m_transmitting;
  bool m_tune;
  bool m_autoCQWatchdogPending;
  bool m_tx_watchdog;           // true when watchdog triggered
  bool m_block_pwr_tooltip;
  bool m_PwrBandSetOK;
  bool m_bDisplayedOnce;
  Frequency m_lastMonitoredFrequency;
  double m_toneSpacing;
  int m_firstDecode;
  QProgressDialog m_optimizingProgress;
  QTimer m_heartbeat;
  MessageClient * m_messageClient;
  PSKReporter m_psk_Reporter;
  DisplayManual m_manual;
  QHash<QString, QVariant> m_pwrBandTxMemory; // Remembers power level by band
  QHash<QString, QVariant> m_pwrBandTuneMemory; // Remembers power level by band for tuning
  QByteArray m_geometryNoControls;
  QVector<double> m_phaseEqCoefficients;
  bool m_block_udp_status_updates;
  bool m_bandActivityRawView = false;

  //---------------------------------------------------- private functions
  // Z 

  void qrzLookup(QString dxCall);
  void qrzVisible(bool b);
  void clearCallInfo();
  QString  stateLookup(QString callsign);
  QString leftJustifyAppendage (QString message, QString appendage);
  void updateBandActivityTitleLabel();
  QString formatRawViewLine(DecodedText const& decodedtext0, QString const& deCall);
  void clearRXWindows();

  void readSettings();
  void set_application_font (QFont const&);
  void setDecodedTextFont (QFont const&);
  void writeSettings();
  void createStatusBar();
  void update_mode_switch_status_label();
  void update_auto_mode_switch_widget();
  void updateStatusBar();
  void genStdMsgs(QString rpt, bool unconditional = false);
  void genCQMsg();
  void clearDX ();
  void lookup();
  void ba2msg(QByteArray ba, char* message);
  void msgtype(QString t, QLineEdit* tx);
  void stub();
  void statusChanged();
  void fixStop();
  bool shortList(QString callsign) const;
  void transmit (double snr = 99.);
  void rigFailure (QString const& reason);
  void pskSetLocal ();
  void pskPost(DecodedText const& decodedtext);
  void displayDialFrequency ();
  void transmitDisplay (bool);
  void processMessage(DecodedText const& message, Qt::KeyboardModifiers = Qt::NoModifier);
  void replyToCQ (QTime, qint32 snr, float delta_time, quint32 delta_frequency, QString const& mode, QString const& message_text, bool low_confidence, quint8 modifiers);
  void locationChange(QString const& location);
  void replayDecodes ();
  void postDecode (bool is_new, QString const& message);
  void postWSPRDecode (bool is_new, QStringList message_parts);
  void enable_DXCC_entity (bool on);
  void switch_mode (Mode);
  void WSPR_scheduling ();
  void freqCalStep();
  void setRig (Frequency = 0);  // zero frequency means no change
  void WSPR_history(Frequency dialFreq, int ndecodes);
  QString beacon_start_time (int n = 0);
  QString WSPR_message();
  void fast_config(bool b);
  void CQTxFreq();
  void useNextCall();
  void abortQSO();
  void updateRate();
  void write_all(QString txRx, QString message);
  bool isWorked(int itype, QString key, float fMHz=0, QString="");

  QString save_wave_file (QString const& name
                          , short const * data
                          , int samples
                          , QString const& my_callsign
                          , QString const& my_grid
                          , QString const& mode
                          , qint32 sub_mode
                          , Frequency frequency
                          , QString const& his_call
                          , QString const& his_grid) const;
  void hound_reply ();
  QString sortHoundCalls(QString t, int isort, int max_dB);
  void rm_tb4(QString houndCall);
  void read_wav_file (QString const& fname);
  void decodeDone ();
  bool subProcessFailed (QProcess *, int exit_code, QProcess::ExitStatus);
  void subProcessError (QProcess *, QProcess::ProcessError);
  void statusUpdate () const;
  void update_watchdog_label ();
  void reset_watchdog_on_click ();
  void invalidate_frequencies_filter ();
  void on_the_minute ();
  void add_child_to_event_filter (QObject *);
  void remove_child_from_event_filter (QObject *);
  void setup_status_bar (bool vhf);
  void tx_watchdog (bool triggered);
  qint64  nWidgets(QString t);
  void displayWidgets(qint64 n);
  QChar current_submode () const; // returns QChar {0} if submode is not appropriate
  void write_transmit_entry (QString const& file_name);
  void selectHound(QString t, bool bTopQueue);
  void removeHoundFromCallingList(QString callsign);
  void houndCallers();
  void updateFoxQSOsInProgressDisplay();
  void foxQueueTopCallCommand();
  void foxRxSequencer(QString msg, QString houndCall, QString rptRcvd);
  void foxTxSequencer();
  void foxGenWaveform(int i,QString fm);
  void writeFoxQSO (QString const& msg);
  void update_foxLogWindow_rate();
  void to_jt9(qint32 n, qint32 istart, qint32 idone);
  bool is77BitMode () const;
  void cease_auto_Tx_after_QSO ();
  Q_SLOT void ARRL_Digi_Display();
  void ARRL_Digi_Update(DecodedText dt);
  void activeWorked(QString call, QString band);
  void read_log();
  void refreshPileupList();
  QString userAgent();
  void handleVerifyMsg(int status, QDateTime ts, QString callsign, QString code, unsigned int hz, QString const &response);
  void writeFoxTxMsgs();
#ifdef FOX_OTP
  QString foxOTPcode();
#endif
};

extern int killbyname(const char* progName);
extern void getDev(int* numDevices,char hostAPI_DeviceName[][50],
                   int minChan[], int maxChan[],
                   int minSpeed[], int maxSpeed[]);
extern int next_tx_state(int pctx);

#endif // MAINWINDOW_H
