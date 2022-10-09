#include <QtWidgets>

#include "dialog.h"


Dialog::Dialog(const std::string& path, QWidget *parent)
    : QWidget(parent)
    , thread(new WokerThread(path, this))
    , channelIndex(0)
    , cmdIndex(0)
    , rangeIndex(0)
{
    QVBoxLayout *verticalLayout;
    if (QGuiApplication::styleHints()->showIsFullScreen() || QGuiApplication::styleHints()->showIsMaximized()) {
        QHBoxLayout *horizontalLayout = new QHBoxLayout(this);
        QGroupBox *groupBox = new QGroupBox(QGuiApplication::applicationDisplayName(), this);
        horizontalLayout->addWidget(groupBox);
        verticalLayout = new QVBoxLayout(groupBox);
    } else 
        verticalLayout = new QVBoxLayout(this);
    

    QToolBox *toolbox = new QToolBox;
    verticalLayout->addWidget(toolbox);

    
    QTimer *timer = new QTimer(this);
    
    

    cmdCombo = new QComboBox();
    
    for(auto& n: cmds)
        cmdCombo->addItem(tr(n.data()));


    QComboBox *cmdChannel = new QComboBox();
    
    QString str;
    for(size_t i = 0; i < voltio::NumberOfChannels; ++i)
    {
        str = "channel";
        str+=  std::to_string(i).c_str();
        cmdChannel->addItem(tr(qPrintable(str)));

        periodicCmds[i][0] = false;
        periodicCmds[i][1] = false;
    }
    
    cmdRange = new QComboBox();
    cmdRange->addItem(tr("range0"));
    cmdRange->addItem(tr("range1"));
    cmdRange->addItem(tr("range2"));
    cmdRange->addItem(tr("range3"));
    cmdRange->hide();

    periodicCheckButton = new QCheckBox (tr("run periodically"));
    

    
    startStopButton = new QPushButton(tr("Start"));

    logArea = new QPlainTextEdit();
    logArea->setReadOnly(true);

    connect(cmdCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(cmdChanged(int)) );
    connect(cmdChannel, SIGNAL(currentIndexChanged(int)), this, SLOT(channelChanged(int)) );
    connect(cmdRange, SIGNAL(currentIndexChanged(int)), this, SLOT(rangeChanged(int)) );
    //connect(periodicCheckButton, SIGNAL(stateChanged(int)), this, SLOT(onRunPeriodically(int)) );
    connect(startStopButton, &QAbstractButton::clicked, this, &Dialog::startStopCmd);  

    // timer
    connect(timer, &QTimer::timeout, this, &Dialog::onTimer);
    
    //! [set up widgets and connections] //! [connecting signal with custom type]
    connect( thread, &WokerThread::sendResponse,this, &Dialog::addResponse);
    connect( thread, &WokerThread::sendFatalError,this, &Dialog::recvFatalError);
    //! [connecting signal with custom type]



    QWidget *page = new QWidget;
    //page->setFixedHeight(70);

    QBoxLayout *_layout = new QBoxLayout( QBoxLayout::LeftToRight, page);

    _layout->addWidget(cmdCombo);
    _layout->addWidget(cmdChannel);
    _layout->addWidget(cmdRange);
    _layout->addWidget(cmdRange);
    _layout->addWidget(periodicCheckButton);
    _layout->addWidget(startStopButton);

    toolbox->addItem(page, tr("Voltmeter Control"));

    toolbox->setFixedHeight(150);

    verticalLayout->addWidget(logArea);


    setWindowTitle(QGuiApplication::applicationDisplayName());

    this->setMinimumWidth(page->width() + 30);

    timer->start(3000); // 1 sec

    periodicCheckButton->hide();

    /*!
        BUGFUX: call start of woker thread only
        after setup signals and slots in connect method!
    */
    thread->start();
}



void Dialog::cmdChanged(int idx)
{
    
    if(0 == cmdCombo->itemText(idx).compare(voltio::set_range::name))
        cmdRange->show();
    else
        cmdRange->hide();

    if (idx > 2)
        periodicCheckButton->show();
    else    
        periodicCheckButton->hide();

    cmdIndex = idx;
    

    syncButtons();
}

void Dialog::channelChanged(int idx)
{
    channelIndex = idx;

    syncButtons();
}

void Dialog::rangeChanged(int idx)
{

    rangeIndex = idx;
}



void Dialog::startStopCmd()
{
    if (cmdIndex > 2)
        if (periodicCheckButton->isChecked()) {
            // periodic commands or not
            if(cmdIndex > 2) {

                // change periodic commands (proseccing->stop or stopped->start periodic command)
                periodicCmds[channelIndex][cmdIndex - 3] = !periodicCmds[channelIndex][cmdIndex - 3]; 

                // change button name 
                (periodicCmds[channelIndex][cmdIndex - 3]) ?
                    startStopButton->setText("Stop") :
                    startStopButton->setText("Start");

                //! NOTE: start periodic command will be done in onTimer not here 
                // according periodicCmds array
            }
            return;
        }
        
    // start non periodic command
    runCommand( createByIndex(cmdIndex));
}   



void Dialog::syncButtons()
{

    // find existing periodic calls
    if(cmdIndex > 2) {

        if (periodicCmds[channelIndex][cmdIndex - 3]){
            // we already have periodically command
            startStopButton->setText("Stop");       
            
            // setup periodically check button 
            periodicCheckButton->setChecked(true);
            // read only
            periodicCheckButton->setAttribute(Qt::WA_TransparentForMouseEvents, true);
            periodicCheckButton->setFocusPolicy(true ? Qt::NoFocus : Qt::StrongFocus);
        }
        else {
            startStopButton->setText("Start");
            // setup periodically check button to unchecked status
            periodicCheckButton->setChecked(false);
            // read only
            periodicCheckButton->setAttribute(Qt::WA_TransparentForMouseEvents, false);
            periodicCheckButton->setFocusPolicy(false ? Qt::NoFocus : Qt::StrongFocus);
        } 
        
    }
    else  // for non periodic cmds the Start button is allways "Start"
        startStopButton->setText("Start");
}


std::unique_ptr<voltio::command> Dialog::createByIndex(int cmdIndex)
{
    std::unique_ptr<voltio::command> cmd;
    switch(cmdIndex){

        case 0: 
            cmd = std::move(std::make_unique<voltio::start_measure_cmd>( channelIndex, nullptr)); break;
        case 1: 
            cmd = std::move(std::make_unique<voltio::stop_measure_cmd>( channelIndex, nullptr)); break;
        case 2: 
            cmd = std::move(
                std::make_unique<voltio::set_range_cmd>( 
                    channelIndex, 
                    voltio::to_range(static_cast<size_t>(rangeIndex)), 
                    nullptr
                    )
                ); 
                break;
        case 3: 
            cmd = std::move(std::make_unique<voltio::get_status_cmd>( channelIndex, nullptr)); break;
        case 4: 
            cmd = std::move(std::make_unique<voltio::get_result_cmd>( channelIndex, nullptr)); break;
    }

    return cmd;
}



void Dialog::runCommand(std::unique_ptr<voltio::command>&& cmd)
{
    
    
    if(nullptr == cmd.get()) return;
    
    std::string ser;
    cmd->serialize(ser);
    // display command
    logArea->appendPlainText(ser.c_str());

    // add to queue
    thread->runCommand(std::move(cmd));
}


void Dialog::onTimer() {

    // we send periodicaly only get_status and get_result
    std::unique_ptr<voltio::command> cmd;

    for(size_t i = 0; i < voltio::NumberOfChannels; ++i) {

        for(size_t j = 0; j < 2; j++)
            if (periodicCmds[i][j]) {
                if (!j)    
                    runCommand(
                        std::move(std::make_unique<voltio::get_status_cmd>( i, nullptr)));
                else
                    runCommand(
                        std::move(std::make_unique<voltio::get_result_cmd>( i, nullptr)));
                
            }
        std::this_thread::yield();
    }
    
}


//! [Adding response and errors from woker thread]
void Dialog::addResponse(const interThreadData& info)
{
    logArea->appendPlainText(info.str);
}

void Dialog::recvFatalError(const interThreadData& err)
{
    QMessageBox messageBox;
    messageBox.critical(0,"Crirical error", err.str);
    messageBox.setFixedSize(500,200);

    close();
}


