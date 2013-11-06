/**
* Copyright (C) 2012-2013 Phonations
* License: http://www.gnu.org/licenses/gpl.html GPL version 2 or higher
*/

#ifndef PREFPANEL_H
#define PREFPANEL_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class PrefPanel;
}

class PrefPanel : public QDialog
{
	Q_OBJECT

public:
	explicit PrefPanel(QSettings *settings, QWidget *parent = 0);
	~PrefPanel();

private slots:

	void on_spinBoxDelay_valueChanged(int delay);
	void on_cBoxQuarterFram_toggled(bool checked);
	void on_sliderStripHeight_valueChanged(int position);
	void on_cBoxSonyAutoconnect_toggled(bool checked);
	void on_spinBoxSpeed_valueChanged(int speed);


	void on_buttonBox_accepted();

	void on_buttonBox_rejected();

	void on_cBoxLastFile_toggled(bool checked);

	void on_cBoxFullscreen_toggled(bool checked);

	void on_sliderBoldness_valueChanged(int value);

private:
	Ui::PrefPanel *ui;
	QSettings *_settings;
	bool _oldUseQuarterFrame;
	int _oldDelay;
	int _oldSpeed;
	int _oldBolness;
	float _oldStripHeight;
	bool _oldSonyAutoConnect;
	bool _oldOpenLastFile;
	bool _oldStartFullScreen;

};

#endif // PREFPANEL_H
