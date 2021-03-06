// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "DolphinQt2/NetPlay/NetPlaySetupDialog.h"

#include "Core/Config/NetplaySettings.h"
#include "DolphinQt2/GameList/GameListModel.h"
#include "DolphinQt2/Settings.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>

NetPlaySetupDialog::NetPlaySetupDialog(QWidget* parent)
    : QDialog(parent), m_game_list_model(Settings::Instance().GetGameListModel())
{
  setWindowTitle(tr("Dolphin NetPlay Setup"));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  CreateMainLayout();

  std::string nickname = Config::Get(Config::NETPLAY_NICKNAME);
  std::string traversal_choice = Config::Get(Config::NETPLAY_TRAVERSAL_CHOICE);
  int connect_port = Config::Get(Config::NETPLAY_CONNECT_PORT);
  int host_port = Config::Get(Config::NETPLAY_HOST_PORT);
  int host_listen_port = Config::Get(Config::NETPLAY_LISTEN_PORT);
#ifdef USE_UPNP
  bool use_upnp = Config::Get(Config::NETPLAY_USE_UPNP);

  m_host_upnp->setChecked(use_upnp);
#endif

  m_nickname_edit->setText(QString::fromStdString(nickname));
  m_connection_type->setCurrentIndex(traversal_choice == "direct" ? 0 : 1);
  m_connect_port_box->setValue(connect_port);
  m_host_port_box->setValue(host_port);

  m_host_force_port_check->setChecked(false);
  m_host_force_port_box->setValue(host_listen_port);
  m_host_force_port_box->setEnabled(false);

  OnConnectionTypeChanged(m_connection_type->currentIndex());

  int selected_game = Settings::GetQSettings().value(QStringLiteral("netplay/hostgame"), 0).toInt();

  if (selected_game >= m_host_games->count())
    selected_game = 0;

  m_host_games->setCurrentItem(m_host_games->item(selected_game));

  ConnectWidgets();
}

void NetPlaySetupDialog::CreateMainLayout()
{
  m_main_layout = new QGridLayout;
  m_button_box = new QDialogButtonBox(QDialogButtonBox::Cancel);
  m_nickname_edit = new QLineEdit;
  m_connection_type = new QComboBox;
  m_reset_traversal_button = new QPushButton(tr("Reset Traversal Settings"));
  m_tab_widget = new QTabWidget;

  // Connection widget
  auto* connection_widget = new QWidget;
  auto* connection_layout = new QGridLayout;

  m_ip_label = new QLabel;
  m_ip_edit = new QLineEdit;
  m_connect_port_label = new QLabel(tr("Port:"));
  m_connect_port_box = new QSpinBox;
  m_connect_button = new QPushButton(tr("Connect"));

  m_connect_port_box->setMaximum(65535);

  connection_layout->addWidget(m_ip_label, 0, 0);
  connection_layout->addWidget(m_ip_edit, 0, 1);
  connection_layout->addWidget(m_connect_port_label, 0, 2);
  connection_layout->addWidget(m_connect_port_box, 0, 3);
  connection_layout->addWidget(
      new QLabel(tr(
          "ALERT:\n\n"
          "All players must use the same Dolphin version.\n"
          "All memory cards, SD cards and cheats must be identical between players or disabled.\n"
          "If DSP LLE is used, DSP ROMs must be identical between players.\n"
          "If connecting directly, the host must have the chosen UDP port open/forwarded!\n"
          "\n"
          "Wii Remote support in netplay is experimental and should not be expected to work.\n")),
      1, 0, -1, -1);
  connection_layout->addWidget(m_connect_button, 3, 3, Qt::AlignRight);

  connection_widget->setLayout(connection_layout);

  // Host widget
  auto* host_widget = new QWidget;
  auto* host_layout = new QGridLayout;
  m_host_port_label = new QLabel(tr("Port:"));
  m_host_port_box = new QSpinBox;
  m_host_force_port_check = new QCheckBox(tr("Force Listen Port:"));
  m_host_force_port_box = new QSpinBox;

#ifdef USE_UPNP
  m_host_upnp = new QCheckBox(tr("Forward port (UPnP)"));
#endif
  m_host_games = new QListWidget;
  m_host_button = new QPushButton(tr("Host"));

  m_host_port_box->setMaximum(65535);
  m_host_force_port_box->setMaximum(65535);

  host_layout->addWidget(m_host_port_label, 0, 0);
  host_layout->addWidget(m_host_port_box, 0, 1);
#ifdef USE_UPNP
  host_layout->addWidget(m_host_upnp, 0, 2);
#endif
  host_layout->addWidget(m_host_games, 1, 0, 1, -1);
  host_layout->addWidget(m_host_force_port_check, 2, 0);
  host_layout->addWidget(m_host_force_port_box, 2, 1, Qt::AlignLeft);
  host_layout->addWidget(m_host_button, 2, 2, Qt::AlignRight);

  host_widget->setLayout(host_layout);

  m_connection_type->addItem(tr("Direct Connection"));
  m_connection_type->addItem(tr("Traversal Server"));

  m_main_layout->addWidget(new QLabel(tr("Connection Type:")), 0, 0);
  m_main_layout->addWidget(m_connection_type, 0, 1);
  m_main_layout->addWidget(m_reset_traversal_button, 0, 2);
  m_main_layout->addWidget(new QLabel(tr("Nickname:")), 1, 0);
  m_main_layout->addWidget(m_nickname_edit, 1, 1);
  m_main_layout->addWidget(m_tab_widget, 2, 0, 1, -1);
  m_main_layout->addWidget(m_button_box, 3, 0, 1, -1);

  // Tabs
  m_tab_widget->addTab(connection_widget, tr("Connect"));
  m_tab_widget->addTab(host_widget, tr("Host"));

  setLayout(m_main_layout);
}

void NetPlaySetupDialog::ConnectWidgets()
{
  connect(m_connection_type, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &NetPlaySetupDialog::OnConnectionTypeChanged);
  connect(m_nickname_edit, &QLineEdit::textChanged, this, &NetPlaySetupDialog::SaveSettings);

  // Connect widget
  connect(m_ip_edit, &QLineEdit::textChanged, this, &NetPlaySetupDialog::SaveSettings);
  connect(m_connect_port_box, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
          &NetPlaySetupDialog::SaveSettings);
  // Host widget
  connect(m_host_port_box, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
          &NetPlaySetupDialog::SaveSettings);
  connect(m_host_games, static_cast<void (QListWidget::*)(int)>(&QListWidget::currentRowChanged),
          [](int index) {
            Settings::GetQSettings().setValue(QStringLiteral("netplay/hostgame"), index);
          });
  connect(m_host_force_port_check, &QCheckBox::toggled,
          [this](int value) { m_host_force_port_box->setEnabled(value); });
#ifdef USE_UPNP
  connect(m_host_upnp, &QCheckBox::stateChanged, this, &NetPlaySetupDialog::SaveSettings);
#endif

  connect(m_connect_button, &QPushButton::clicked, this, &QDialog::accept);
  connect(m_host_button, &QPushButton::clicked, this, &QDialog::accept);
  connect(m_button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(m_reset_traversal_button, &QPushButton::clicked, this,
          &NetPlaySetupDialog::ResetTraversalHost);
}

void NetPlaySetupDialog::SaveSettings()
{
  Config::SetBaseOrCurrent(Config::NETPLAY_NICKNAME, m_nickname_edit->text().toStdString());
  Config::SetBaseOrCurrent(Config::NETPLAY_HOST_CODE, m_ip_edit->text().toStdString());
  Config::SetBaseOrCurrent(Config::NETPLAY_CONNECT_PORT,
                           static_cast<u16>(m_connect_port_box->value()));
  Config::SetBaseOrCurrent(Config::NETPLAY_HOST_PORT, static_cast<u16>(m_host_port_box->value()));
#ifdef USE_UPNP
  Config::SetBaseOrCurrent(Config::NETPLAY_USE_UPNP, m_host_upnp->isChecked());
#endif

  if (m_host_force_port_check->isChecked())
    Config::SetBaseOrCurrent(Config::NETPLAY_LISTEN_PORT,
                             static_cast<u16>(m_host_force_port_box->value()));
}

void NetPlaySetupDialog::OnConnectionTypeChanged(int index)
{
  m_connect_port_box->setHidden(index != 0);
  m_connect_port_label->setHidden(index != 0);

  m_host_port_label->setHidden(index != 0);
  m_host_port_box->setHidden(index != 0);
#ifdef USE_UPNP
  m_host_upnp->setHidden(index != 0);
#endif
  m_host_force_port_check->setHidden(index == 0);
  m_host_force_port_box->setHidden(index == 0);

  m_reset_traversal_button->setHidden(index == 0);

  std::string address = Config::Get(Config::NETPLAY_HOST_CODE);

  m_ip_label->setText(index == 0 ? tr("IP Address:") : tr("Host Code:"));
  m_ip_edit->setText(QString::fromStdString(address));

  Config::SetBaseOrCurrent(Config::NETPLAY_TRAVERSAL_CHOICE,
                           std::string(index == 0 ? "direct" : "traversal"));
}

void NetPlaySetupDialog::show()
{
  PopulateGameList();
  QDialog::show();
}

void NetPlaySetupDialog::accept()
{
  SaveSettings();
  if (m_tab_widget->currentIndex() == 0)
  {
    emit Join();
  }
  else
  {
    auto items = m_host_games->selectedItems();
    if (items.size() == 0)
    {
      QMessageBox::critical(this, tr("Error"), tr("You must select a game to host!"));
      return;
    }

    emit Host(items[0]->text());
  }
}

void NetPlaySetupDialog::PopulateGameList()
{
  m_host_games->clear();
  for (int i = 0; i < m_game_list_model->rowCount(QModelIndex()); i++)
  {
    auto title = m_game_list_model->GetUniqueIdentifier(i);
    auto path = m_game_list_model->GetPath(i);

    auto* item = new QListWidgetItem(title);
    item->setData(Qt::UserRole, path);
    m_host_games->addItem(item);
  }

  m_host_games->sortItems();
}

void NetPlaySetupDialog::ResetTraversalHost()
{
  Config::SetBaseOrCurrent(Config::NETPLAY_TRAVERSAL_SERVER,
                           Config::NETPLAY_TRAVERSAL_SERVER.default_value);
  Config::SetBaseOrCurrent(Config::NETPLAY_TRAVERSAL_PORT,
                           Config::NETPLAY_TRAVERSAL_PORT.default_value);

  QMessageBox::information(
      this, tr("Reset Traversal Server"),
      tr("Reset Traversal Server to %1:%2")
          .arg(QString::fromStdString(Config::NETPLAY_TRAVERSAL_SERVER.default_value),
               QString::number(Config::NETPLAY_TRAVERSAL_PORT.default_value)));
}
