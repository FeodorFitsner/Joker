#ifndef PHSONYCONTROLLER_H
#define PHSONYCONTROLLER_H

#include <QObject>
#include <QSerialPort>

#include <../../libs/PhTools/PhTime.h>

/**
 * PhSonyController is an abstract class handle sony 9 pin communication
 * through the serial port.
 * It provide a generic implementation for handlind sony master
 * and slave communication.
 * It can be connected to a clock to update it accordingly.
 */
class PhSonyController : public QObject
{
	Q_OBJECT
public:
	/**
	 * @brief PhSonyController constructor
	 * @param comSuffix Serial port name suffix
	 * @param parent Parent for the QObject (mandatory)
	 */
	explicit PhSonyController(QString comSuffix, QObject *parent);

	/**
	 Start the thread handling the communication.
	 */
	bool start();

	/**
	 Stop the thread handling the communication.
	 */
	void stop();

	void test();

protected:
	/**
	 * Compute the rate from the jog, varispeed and shuttle sony protocole
	 * order data.
	 * For more detail see http://www.belle-nuit.com/archives/9pin.html#jogFwd
	 * @param data1 A one byte coded version of the rate.
	 * @return The float value corresponding rate.
	 */
	PhRate computeRate(unsigned char data1);

	/**
	 * Compute the rate from the jog, varispeed and shuttle sony protocole
	 * order data.
	 * For more detail see http://www.belle-nuit.com/archives/9pin.html#jogFwd
	 * @param data1 The first byte of the two bytes coded version of the rate.
	 * @param data2 The second byte of the two bytes coded version of the rate.
	 * @return The float value corresponding rate.
	 */
	PhRate computeRate(unsigned char data1, unsigned char data2);

	/**
	 * Compute the jog, varispeed and shuttle sony protocole
	 * order data from a rate
	 * For more detail see http://www.belle-nuit.com/archives/9pin.html#jogFwd
	 * @param rate The float value rate.
	 * @return A one byte coded version of the rate.
	 */
	unsigned char computeData1(PhRate rate);

	/**
	 * Get an element of the device status.
	 * For more detail see http://www.belle-nuit.com/archives/9pin.html#statusData
	 * @param index Index of the status array
	 * @return A 8 bit status information
	 */
	unsigned char status(int index);

	/**
	 * Process a single command and respond to it, updating the clock if needed.
	 * This method shall be implemented differently by the slave and the master.
	 * @param cmd1 TODO
	 * @param cmd2 TODO
	 * @param dataIn Command data.
	 */
	virtual void processCommand(unsigned char cmd1, unsigned char cmd2, const unsigned char* data) = 0;

private:
	/** @brief PhSonyController destructor
	 *
	 * Closing the port if open.
	 * /
	~PhSonyController();


	/** Serial port connected to the controller */
	QSerialPort _serial;

	/** Serial port name suffix (A for slave and B for master) */
	QString _comSuffix;

	/** Clock linked to the controller */
	//PhClock * clock;	TODO: get julien implementation

	/** Sony controller status */
	unsigned char _status[8];

private slots:
	void onData();
	void onCTS();
	void handleError(QSerialPort::SerialPortError error);
};

#endif // PHSONYCONTROLLER_H

