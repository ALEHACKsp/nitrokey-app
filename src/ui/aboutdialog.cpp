/*
 * Author: Copyright (C)  Rudolf Boeddeker  Date: 2014-04-12
 *
 * This file is part of Nitrokey.
 *
 * Nitrokey is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * Nitrokey is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nitrokey. If not, see <http://www.gnu.org/licenses/>.
 */

#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "stick20infodialog.h"
#include "stick20responsedialog.h"

AboutDialog::AboutDialog (Device * global_cryptostick, QWidget * parent):
QDialog (parent), ui (new Ui::AboutDialog)
{
    ui->setupUi (this);

    cryptostick = global_cryptostick;

QPixmap image (":/images/splash.png");

QPixmap small_img = image.scaled (346, 100, Qt::KeepAspectRatio, Qt::FastTransformation);

    // cryptostick->getStatus();
int majorFirmwareVersion = cryptostick->firmwareVersion[0] / 10;

int minorFirmwareVersion = cryptostick->firmwareVersion[0] % 10;

    ui->IconLabel->setPixmap (small_img);
    ui->VersionLabel->setText (tr (GUI_VERSION));
    ui->firmwareLabel->setText (QString::number (majorFirmwareVersion).append (".").append (QString::number (minorFirmwareVersion)));

QByteArray cardSerial = QByteArray ((char *) cryptostick->cardSerial).toHex ();
    ui->serialEdit->setText (QString ("%1").arg (QString (cardSerial), 8, '0'));

    ui->ButtonStickStatus->hide ();

    if (true == cryptostick->isConnected)
    {
        if (TRUE == cryptostick->activStick20)
        {
            showStick20Configuration ();
        }
        else
        {
            showStick10Configuration ();
            ui->ButtonStickStatus->hide ();
        }
    }
    else
    {
        showNoStickFound ();
        // ui->ButtonStickStatus->hide();
    }


}

AboutDialog::~AboutDialog ()
{
delete ui;
}

void AboutDialog::on_ButtonOK_clicked ()
{
    done (TRUE);
}



/*******************************************************************************

  showStick20Configuration

  Changes
  Date      Author        Info
  02.07.14  RB            Function created

  Reviews
  Date      Reviewer        Info

*******************************************************************************/

void AboutDialog::showStick20Configuration (void)
{
    QString OutputText;

    bool ErrorFlag;

    ErrorFlag = FALSE;

    if (0 == HID_Stick20Configuration_st.ActiveSD_CardID_u32)
    {
        OutputText.append (QString (tr ("\nSD card is not accessible\n\n")));
        ErrorFlag = TRUE;
    }

    if (0 == HID_Stick20Configuration_st.ActiveSmartCardID_u32)
    {
        ui->serialEdit->setText ("-");
        OutputText.append (QString (tr ("\nSmartcard is not accessible\n\n")));
        ErrorFlag = TRUE;
    }

    if (TRUE == ErrorFlag)
    {
        ui->DeviceStatusLabel->setText (OutputText);
        return;
    }

    if (99 == HID_Stick20Configuration_st.UserPwRetryCount)
    {
        OutputText.append (QString (tr ("No connection\nPlease retry")));
        ui->DeviceStatusLabel->setText (OutputText);
        return;
    }

    if (TRUE == HID_Stick20Configuration_st.StickKeysNotInitiated)
    {
        OutputText.append (QString (tr (" ***  Warning stick is not securce  ***")).append ("\n"));
        OutputText.append (QString (tr (" ***        Select -Init keys-      ***")).append ("\n").append ("\n"));
    }


    if (TRUE == HID_Stick20Configuration_st.FirmwareLocked_u8)
    {
        OutputText.append (QString (tr ("      *** Firmware is locked *** ")).append ("\n"));
    }

    if (0 != (HID_Stick20Configuration_st.NewSDCardFound_u8 & 0x01))
    {
        OutputText.append (QString (tr ("      *** New SD card found  ***\n")));
    }

    if (0 == (HID_Stick20Configuration_st.SDFillWithRandomChars_u8 & 0x01))
    {
        OutputText.append (QString (tr (" *** Not erased with random chars ***\n")));
    }


    if (READ_WRITE_ACTIVE == HID_Stick20Configuration_st.ReadWriteFlagUncryptedVolume_u8)
    {
        OutputText.append (QString (tr ("Unencrypted volume   READ/WRITE mode ")).append ("\n"));
    }
    else
    {
        OutputText.append (QString (tr ("Unencrypted volume   READ ONLY mode ")).append ("\n"));
    }

    if (0 != (HID_Stick20Configuration_st.VolumeActiceFlag_u8 & (1 << SD_HIDDEN_VOLUME_BIT_PLACE)))
    {
        OutputText.append (QString (tr ("Hidden volume        active")).append ("\n"));
    }
    else
    {
        if (0 != (HID_Stick20Configuration_st.VolumeActiceFlag_u8 & (1 << SD_CRYPTED_VOLUME_BIT_PLACE)))
        {
            OutputText.append (QString (tr ("Encrypted volume     active")).append ("\n"));
        }
        else
        {
            OutputText.append (QString (tr ("Encrypted volume     not active")).append ("\n"));
        }
    }

    OutputText.append (QString ("\n"));

    OutputText.append (QString (tr ("PIN retry counter\n")));
    OutputText.append (QString (tr ("Admin : ")));
    OutputText.append (QString ("%1").arg (QString::number (HID_Stick20Configuration_st.AdminPwRetryCount))).append ("\n");
    OutputText.append (QString (tr ("User  : ")));
    OutputText.append (QString ("%1").arg (QString::number (HID_Stick20Configuration_st.UserPwRetryCount))).append ("\n");


    OutputText.append (QString ("\n"));
    OutputText.append (QString ("SD ID 0x"));
    OutputText.append (QString ("%1").arg (QString::number (HID_Stick20Configuration_st.ActiveSD_CardID_u32, 16))).append ("\n");
    /*
       OutputText.append(QString("SD change counter "));
       OutputText.append(QString("st.NewSDCardFoun::number(HID_Stick20Configuration_st.NewSDCardFound_u8 >> 1))).append("\n");
       OutputText.append(QString("SD erase counter "));
       OutputText.append(QString("%1t.SDFillWithRandnumber(HID_Stick20Configuration_st.SDFillWithRandomChars_u8 >> 1))).append("\n"); */

    ui->DeviceStatusLabel->setText (OutputText);

    ui->firmwareLabel->setText (QString::number (HID_Stick20Configuration_st.VersionInfo_au8[0]).append (".").append
                                (QString::number (HID_Stick20Configuration_st.VersionInfo_au8[1])));

    ui->serialEdit->setText (QString ("%1").sprintf ("%08x", HID_Stick20Configuration_st.ActiveSmartCardID_u32));
}



/*******************************************************************************

  showStick10Configuration

  Changes
  Date      Author        Info
  23.07.14  RB            Function created

  Reviews
  Date      Reviewer        Info

*******************************************************************************/

void AboutDialog::showStick10Configuration (void)
{
    QString OutputText;

    cryptostick->getPasswordRetryCount ();
    cryptostick->getUserPasswordRetryCount ();

    OutputText.append (QString (tr ("PIN retry counter\n")));
    OutputText.append (QString (tr ("Admin : ")));
    OutputText.append (QString ("%1").arg (QString::number (cryptostick->passwordRetryCount))).append ("\n");

    OutputText.append (QString (tr ("User  : ")));
    OutputText.append (QString ("%1").arg (QString::number (cryptostick->userPasswordRetryCount))).append ("\n");

    ui->DeviceStatusLabel->setText (OutputText);
}

/*******************************************************************************

  showNoStickFound

  Changes
  Date      Author        Info
  23.07.14  RB            Function created

  Reviews
  Date      Reviewer        Info

*******************************************************************************/

void AboutDialog::showNoStickFound (void)
{
    QString OutputText;

    OutputText.append (QString (tr ("No active Nitrokey\n\n")));

    ui->DeviceStatusLabel->setText (OutputText);

    ui->firmwareLabel->setText ("-");
    ui->serialEdit->setText ("-");
}

void AboutDialog::on_ButtonStickStatus_clicked ()
{
Stick20InfoDialog InfoDialog (this);

    InfoDialog.exec ();
}
