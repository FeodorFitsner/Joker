#include "VideoTestWindow.h"
#include "ui_VideoTestWindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>

#include "PhCommonUI/PhTimeCodeDialog.h"

VideoTestWindow::VideoTestWindow(VideoTestSettings *settings)
	: PhDocumentWindow(settings),
	ui(new Ui::VideoTestWindow),
	_settings(settings),
	_videoEngine(settings),
	_maxVideoRate(0)
{
	ui->setupUi(this);
	ui->videoView->setGraphicSettings(settings);

	_mediaPanelDialog.setClock(_videoEngine.timeCodeType(), _videoEngine.clock());

	connect(&_videoEngine, &PhVideoEngine::timeCodeTypeChanged, &_mediaPanelDialog, &PhMediaPanel::onTimeCodeTypeChanged);

	ui->actionDisplay_media_panel->setChecked(_settings->displayMediaPanel());
	ui->actionDeinterlace_video->setChecked(_settings->deinterlaceVideo());

	_videoEngine.setDeinterlace(_settings->deinterlaceVideo());

	connect(ui->videoView, &PhGraphicView::paint, this, &VideoTestWindow::onPaint);
	connect(ui->videoView, &PhGraphicView::beforePaint, _videoEngine.clock(), &PhClock::tick);
	connect(_videoEngine.clock(), &PhClock::timeChanged, this, &VideoTestWindow::onTimeChanged);
}

VideoTestWindow::~VideoTestWindow()
{
	delete ui;
}

bool VideoTestWindow::openDocument(QString fileName)
{
	if(!_videoEngine.open(fileName))
		return false;

	_mediaPanelDialog.setLength(_videoEngine.length());
	PhTime timeStamp = _videoEngine.timeIn();
	PhTime currentTime = timeStamp;

	if(fileName == _settings->currentDocument()) {
		timeStamp = _settings->timeStamp();
		_videoEngine.setTimeIn(timeStamp);
		currentTime = _settings->currentTime();
	}
	else if(_videoEngine.timeIn() == 0) {
		on_actionSet_timestamp_triggered();
		timeStamp = _videoEngine.timeIn();
		currentTime = timeStamp;
	}
	_mediaPanelDialog.setTimeIn(timeStamp);

	_videoEngine.clock()->setTime(currentTime);

	setCurrentDocument(fileName);
	_settings->setTimeStamp(timeStamp);

	return true;
}

void VideoTestWindow::processArg(int argc, char *argv[])
{
	PhDocumentWindow::processArg(argc, argv);
	for(int i = 1; i < argc; i++) {
		PhTime timeIn = PhTimeCode::timeFromString(argv[i], _videoEngine.timeCodeType());
		if(timeIn) {
			_videoEngine.setTimeIn(timeIn);
			_videoEngine.clock()->setTime(timeIn);
		}
	}
}

void VideoTestWindow::resizeEvent(QResizeEvent *)
{
	PHDEBUG << this->width() << this->height();
	_mediaPanelDialog.move(this->x() + this->width() / 2 - _mediaPanelDialog.width() / 2,
	                       this->y() + this->height() * 0.95 - _mediaPanelDialog.height());
}

void VideoTestWindow::closeEvent(QCloseEvent *)
{
	_mediaPanelDialog.close();
}

QMenu *VideoTestWindow::recentDocumentMenu()
{
	return ui->menuOpen_recent;
}

void VideoTestWindow::onApplicationActivate()
{
	if(_settings->displayMediaPanel())
		_mediaPanelDialog.show();
}

void VideoTestWindow::onApplicationDeactivate()
{
	_mediaPanelDialog.hide();
}

void VideoTestWindow::on_actionPlay_pause_triggered()
{
	if(_videoEngine.clock()->rate() != 0)
		_videoEngine.clock()->setRate(0);
	else
		_videoEngine.clock()->setRate(1);
}

void VideoTestWindow::on_actionNext_frame_triggered()
{
	_videoEngine.clock()->setTime(_videoEngine.clock()->time() + PhTimeCode::timePerFrame(_videoEngine.timeCodeType()));
}

void VideoTestWindow::on_actionPrevious_frame_triggered()
{
	_videoEngine.clock()->setTime(_videoEngine.clock()->time() - PhTimeCode::timePerFrame(_videoEngine.timeCodeType()));
}

void VideoTestWindow::on_actionSet_timestamp_triggered()
{
	_mediaPanelDialog.hide();
	PhTimeCodeType tcType = _videoEngine.timeCodeType();
	PhTime time;
	if(_videoEngine.clock()->time() < _videoEngine.timeIn())
		time = _videoEngine.timeIn();
	else if(_videoEngine.clock()->time() > _videoEngine.timeIn() + _videoEngine.length())
		time = _videoEngine.timeOut();
	else
		time = _videoEngine.clock()->time();

	PhTimeCodeDialog dlg(_videoEngine.timeCodeType(), time);
	if(dlg.exec() == QDialog::Accepted) {
		PhTime timeStamp;
		if(_videoEngine.clock()->time() > _videoEngine.timeIn() + _videoEngine.length())
			timeStamp = dlg.time() - (_videoEngine.length() - PhTimeCode::timePerFrame(tcType));
		else if (_videoEngine.clock()->time() < _videoEngine.timeIn())
			timeStamp =  dlg.time();
		else
			timeStamp = _videoEngine.timeIn() + dlg.time() - _videoEngine.clock()->time();

		_videoEngine.setTimeIn(timeStamp);
		_mediaPanelDialog.setTimeIn(timeStamp);
		_videoEngine.clock()->setTime(dlg.time());
		_settings->setTimeStamp(timeStamp);
	}

	if(_settings->displayMediaPanel())
		_mediaPanelDialog.show();
}

void VideoTestWindow::on_actionOpen_triggered()
{
	_mediaPanelDialog.hide();
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Movie"), _settings->lastDocumentFolder());
	if(QFile::exists(fileName)) {
		if(!openDocument(fileName))
			QMessageBox::critical(this, "Error", "Unable to open " + fileName);
	}

	if(_settings->displayMediaPanel())
		_mediaPanelDialog.show();
}

void VideoTestWindow::on_actionReverse_triggered()
{
	_videoEngine.clock()->setRate(-1);
}

void VideoTestWindow::on_actionGo_to_triggered()
{
	_mediaPanelDialog.hide();
	PhTimeCodeDialog dlg(_videoEngine.timeCodeType(), _videoEngine.clock()->time(), this);

	if(dlg.exec() == QDialog::Accepted) {
		PHDEBUG << PhTimeCode::stringFromTime(dlg.time(), _videoEngine.timeCodeType());
		_videoEngine.clock()->setTime(dlg.time());
	}

	if(_settings->displayMediaPanel())
		_mediaPanelDialog.show();
}

void VideoTestWindow::on_actionDisplay_media_panel_triggered(bool checked)
{
	_settings->setDisplayMediaPanel(checked);
	if(checked)
		_mediaPanelDialog.show();
	else
		_mediaPanelDialog.hide();
}

void VideoTestWindow::on_actionDeinterlace_video_triggered(bool checked)
{
	_settings->setDeinterlaceVideo(checked);
	_videoEngine.setDeinterlace(checked);
}

void VideoTestWindow::onTimeChanged(PhTime time)
{
	_settings->setCurrentTime(time);
	ui->statusBar->showMessage(PhTimeCode::stringFromTime(time, _videoEngine.timeCodeType()));

}

void VideoTestWindow::onPaint(int width, int height)
{
	int videoRate = _videoEngine.refreshRate();
	if(videoRate > _maxVideoRate)
		_maxVideoRate = videoRate;
	QString info = QString("%1 / %2").arg(videoRate).arg(_maxVideoRate);
	ui->videoView->addInfo(info);
	_videoEngine.drawVideo(0, 0, width, height);
}

void VideoTestWindow::updateProgressInformation(PhFrame frame)
{
	PhTimeCodeType tcType = _videoEngine.timeCodeType();
	// Update display every second
	if(frame % PhTimeCode::getFps(tcType) == 0) {
		progressValueChanged(frame);
		progressInformationChanged(PhTimeCode::stringFromFrame(frame, tcType)
		                           + "/" + PhTimeCode::stringFromFrame(_videoEngine.frameOut(), tcType));
	}
}

void VideoTestWindow::on_actionExport_to_MJPEG_triggered()
{
	QString outputFile = _settings->currentDocument().split(".").first() + "_MJPEG.mov";
	outputFile = QFileDialog::getSaveFileName(this, tr("Export..."), outputFile, "*.mov");
	if(!outputFile.isEmpty()) {
		QProgressDialog progressDialog(tr("Export progress :"), tr("Cancel"), _videoEngine.frameIn(), _videoEngine.frameOut(), this);
		connect(&_videoEngine, &PhVideoEngine::frameExported, this, &VideoTestWindow::updateProgressInformation);
		connect(this, &VideoTestWindow::progressValueChanged, &progressDialog, &QProgressDialog::setValue);
		connect(this, &VideoTestWindow::progressInformationChanged, &progressDialog, &QProgressDialog::setLabelText);
		connect(&progressDialog, &QProgressDialog::canceled, &_videoEngine, &PhVideoEngine::cancelExport);
		progressDialog.show();

		if(_videoEngine.exportToMjpeg(outputFile)) {
			int ret = QMessageBox::question(this, tr("Export succeed"), tr("Do you want to open the exported file?"), QMessageBox::Yes | QMessageBox::No);
			if(ret == QMessageBox::Yes) {
				openDocument(outputFile);
			}
		}
		progressDialog.hide();
	}
}


