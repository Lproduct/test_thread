#include "devicedetection.h"

DeviceDetection::DeviceDetection(QWidget *parent):QMainWindow(parent)
{

}

DeviceDetection::~DeviceDetection()
{

}

typedef bool( *SUPPORTED_DEVICE_CHECK )( const mvIMPACT::acquire::Device* const );

void DeviceDetection::getDeviceFromUserInput()
{
    SUPPORTED_DEVICE_CHECK pSupportedDeviceCheckFn = 0;
    bool boSilent = false;
    bool boAutomaticallyUseGenICamInterface = true;

    DeviceManager devMgr;
    const unsigned int devCnt = devMgr.deviceCount();
    if( devCnt == 0 )
    {
        //std::cout << "No compatible device found!" << std::endl;
        emit warning("No compatible device found!");
        //break;
    }

    std::set<unsigned int> validDeviceNumbers;

    QVector<QString> deviceTab;
    // display every device detected that matches
    for( unsigned int i = 0; i < devCnt; i++ )
    {
        Device* pDev = devMgr[i];
        QString strTemp;
        if( pDev )
        {
            if( !pSupportedDeviceCheckFn || pSupportedDeviceCheckFn( pDev ) )
            {
                //strTemp += "[" + QString::number(i) + "]: " + QString::fromStdString(pDev->serial.readS()) + " (" + QString::fromStdString(pDev->product.readS()) + ", " + QString::fromStdString(pDev->family.readS());
                strTemp += "[" + QString::number(i) + "]: " + " (" + QString::fromStdString(pDev->product.readS()) + ", " + QString::fromStdString(pDev->family.readS());
                if( pDev->interfaceLayout.isValid() )
                {
                    if( boAutomaticallyUseGenICamInterface )
                    {
                        // if this device offers the 'GenICam' interface switch it on, as this will
                        // allow are better control over GenICam compliant devices
                        conditionalSetProperty( pDev->interfaceLayout, dilGenICam, true );
                    }
                    //strTemp += ", interface layout: " + QString::fromStdString(pDev->interfaceLayout.readS());
                }
                if( pDev->acquisitionStartStopBehaviour.isValid() )
                {
                    // if this device offers a user defined acquisition start/stop behaviour
                    // enable it as this allows finer control about the streaming behaviour
                    conditionalSetProperty( pDev->acquisitionStartStopBehaviour, assbUser, true );
                    //strTemp += ", acquisition start/stop behaviour: " + QString::fromStdString(pDev->acquisitionStartStopBehaviour.readS());
                }
                if( pDev->interfaceLayout.isValid() && !pDev->interfaceLayout.isWriteable() )
                {
                    if( pDev->isInUse() )
                    {
                        strTemp += ", !!!ALREADY IN USE!!!";
                    }
                }
                strTemp += ") \r\n";
                validDeviceNumbers.insert( i );
            }
        }
        deviceTab.push_back(strTemp);
    }

    if (!deviceTab.isEmpty())
    {
        emit devices(deviceTab);
    }


    if( validDeviceNumbers.empty() )
    {
        QString noDevices;
        noDevices += " devices have been detected:";
        for( unsigned int i = 0; i < devCnt; i++ )
        {
            Device* pDev = devMgr[i];
            if( pDev )
            {
                noDevices += "[" + QString::number(i) + "]: " + QString::fromStdString(pDev->serial.readS()) + " (" + QString::fromStdString(pDev->product.readS()) + ", " + QString::fromStdString(pDev->family.readS()) + ")" + "\r\n";
            }
        }
        noDevices += "However none of these devices seems to be supported by this sample. \r\n \r\n";
        emit warning(noDevices);
        //break;
    }
}
