/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "settingsdialog.h"
#include "ui_settings.h"
#include <QSettings>
#include <qcompleter.h>
#include "mainwindow.h"

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent), ui(new Ui::SettingsDialog) {
    ui->setupUi(this);

    QSettings settings;
    settings.beginGroup("hosts");
    QStringList hosts = settings.childGroups();

    for (int i=0;i<hosts.size();++i) {
        settings.beginGroup(hosts[i]);
        ui->cmbHost->addItem(settings.value("host").toString(), settings.value("port").toInt());
        settings.endGroup();
    }

    //default settings
    ui->cmbHost->setEditText(settings.value ( "host","127.0.0.1" ).toString());
    ui->spinPort->setValue(settings.value ( "port", 3101 ).toInt());
    settings.beginGroup(ui->cmbHost->currentText());
    ui->lineLocalbase->setText(settings.value("baselocal","/media/roomserver/daten").toString());
    ui->lineRemotebase->setText(settings.value("baseremote","/media/daten").toString());

    QCompleter *completer = new QCompleter(hosts, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    ui->cmbHost->setCompleter(completer);
}

void SettingsDialog::on_cmbHost_currentIndexChanged(int) {
    QSettings settings;
    settings.beginGroup("hosts");
    settings.beginGroup(ui->cmbHost->currentText());
    //ui->cmbHost->setEditText(settings.value("host","127.0.0.1").toString());
    ui->spinPort->setValue(settings.value("port","3101").toInt());
    ui->lineLocalbase->setText(settings.value("baselocal","/media/roomserver/daten").toString());
    ui->lineRemotebase->setText(settings.value("baseremote","/media/daten").toString());
}

void SettingsDialog::on_buttonBox_rejected() {
    hide();
}

void SettingsDialog::on_buttonBox_accepted() {
    QSettings settings;
    settings.setValue("baselocal", ui->lineLocalbase->text());
    settings.setValue("baseremote", ui->lineRemotebase->text());
    QString host = ui->cmbHost->currentText();
    int port = ui->spinPort->value();
    settings.beginGroup("hosts");
    settings.setValue("host", host);
    settings.setValue("port", port);
    settings.beginGroup(host);
    settings.setValue("host", host);
    settings.setValue("port", port);
    settings.setValue("baselocal", ui->lineLocalbase->text());
    settings.setValue("baseremote", ui->lineRemotebase->text());
    settings.endGroup();
    settings.endGroup();

    MainWindow* mw = dynamic_cast<MainWindow*>(parent());
    mw->updateConnectionsSettings();

    hide();
}
