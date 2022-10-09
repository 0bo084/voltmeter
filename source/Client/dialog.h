#ifndef DIALOG_H
#define DIALOG_H

#include <QWidget>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QCheckBox>



#include "../command.hpp"
#include "../voltio.hpp"
#include "woker_thread.h"

#include <string>
#include <memory>

/*
QT_BEGIN_NAMESPACE
class QCheckBox;
class QLabel;
class QPushButton;
class QPlainTextEdit;
QT_END_NAMESPACE
*/

class Dialog : public QWidget
{
    Q_OBJECT

public:
    Dialog(const std::string& path, QWidget *parent = nullptr);


public slots:
    void addResponse(const interThreadData&);
    void recvFatalError(const interThreadData&);


private slots:

    void startStopCmd();
    void cmdChanged(int idx);
    void rangeChanged(int idx);
    void channelChanged(int idx);
    void onTimer();
    
private:

    void syncButtons();
    std::unique_ptr<voltio::command> createByIndex(int cmdIndex);
    void runCommand(std::unique_ptr<voltio::command>&& cmd);
    
private:

    static constexpr std::array <std::string_view, 5> cmds {
        voltio::start_measure::name,
        voltio::stop_measure::name,
        voltio::set_range::name,
        voltio::get_status::name,
        voltio::get_result::name
        };
                

    QPushButton*        startStopButton;
    QPlainTextEdit*     logArea;
    QComboBox*          cmdCombo;
    QComboBox*          cmdRange;
    QCheckBox*          periodicCheckButton;

    WokerThread*        thread;

    /*! 
        this array stores value which means run periodicaly 
        get_status and get_result commands 
    */
    std::array<std::array<bool, 2>, voltio::NumberOfChannels> periodicCmds;
    int channelIndex;
    int cmdIndex;
    int rangeIndex;
};

#endif