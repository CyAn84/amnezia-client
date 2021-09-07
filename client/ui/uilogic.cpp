#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHostInfo>
#include <QItemSelectionModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QMetaEnum>
#include <QSysInfo>
#include <QThread>
#include <QTimer>
#include <QRegularExpression>
#include <QSaveFile>

#include "configurators/cloak_configurator.h"
#include "configurators/vpn_configurator.h"
#include "configurators/openvpn_configurator.h"
#include "configurators/shadowsocks_configurator.h"
#include "configurators/ssh_configurator.h"

#include "core/servercontroller.h"
#include "core/server_defs.h"
#include "core/errorstrings.h"

#include "protocols/protocols_defs.h"
#include "protocols/shadowsocksvpnprotocol.h"

#include "ui/qautostart.h"

#include "debug.h"
#include "defines.h"
#include "uilogic.h"
#include "utils.h"
#include "vpnconnection.h"
#include <functional>

#if defined Q_OS_MAC || defined Q_OS_LINUX
#include "ui/macos_util.h"
#endif

#include "pages_logic/AppSettingsLogic.h"
#include "pages_logic/GeneralSettingsLogic.h"
#include "pages_logic/NetworkSettingsLogic.h"
#include "pages_logic/NewServerProtocolsLogic.h"
#include "pages_logic/ProtocolSettingsLogic.h"
#include "pages_logic/ServerListLogic.h"
#include "pages_logic/ServerSettingsLogic.h"
#include "pages_logic/ServerVpnProtocolsLogic.h"
#include "pages_logic/ShareConnectionLogic.h"
#include "pages_logic/SitesLogic.h"
#include "pages_logic/StartPageLogic.h"
#include "pages_logic/VpnLogic.h"
#include "pages_logic/WizardLogic.h"

#include "pages_logic/protocols/CloakLogic.h"
#include "pages_logic/protocols/OpenVpnLogic.h"
#include "pages_logic/protocols/ShadowSocksLogic.h"


using namespace amnezia;
using namespace PageEnumNS;

UiLogic::UiLogic(QObject *parent) :
    QObject(parent),
    m_frameWireguardSettingsVisible{false},
    m_frameWireguardVisible{false},
    m_frameNewServerSettingsParentWireguardVisible{false},

    m_progressBarNewServerConfiguringValue{0},
    m_pushButtonNewServerSettingsCloakChecked{false},
    m_pushButtonNewServerSettingsSsChecked{false},
    m_pushButtonNewServerSettingsOpenvpnChecked{false},
    m_lineEditNewServerCloakPortText{},
    m_lineEditNewServerCloakSiteText{},
    m_lineEditNewServerSsPortText{},
    m_comboBoxNewServerSsCipherText{"chacha20-ietf-poly1305"},
    m_lineEditNewServerOpenvpnPortText{},
    m_comboBoxNewServerOpenvpnProtoText{"udp"},
    m_radioButtonVpnModeAllSitesChecked{true},
    m_radioButtonVpnModeForwardSitesChecked{false},
    m_radioButtonVpnModeExceptSitesChecked{false},
    m_pushButtonVpnAddSiteEnabled{true},



    m_currentPageValue{0},
    m_trayIconUrl{},
    m_trayActionDisconnectEnabled{true},
    m_trayActionConnectEnabled{true},
    m_checkBoxNewServerCloakChecked{true},
    m_checkBoxNewServerSsChecked{false},
    m_checkBoxNewServerOpenvpnChecked{false},




    m_pushButtonConnectChecked{false},


    m_pushButtonProtoOpenvpnContInstallChecked{false},
    m_pushButtonProtoSsOpenvpnContInstallChecked{false},
    m_pushButtonProtoCloakOpenvpnContInstallChecked{false},
    m_pushButtonProtoWireguardContInstallChecked{false},
    m_pushButtonProtoOpenvpnContInstallEnabled{false},
    m_pushButtonProtoSsOpenvpnContInstallEnabled{false},
    m_pushButtonProtoCloakOpenvpnContInstallEnabled{false},
    m_pushButtonProtoWireguardContInstallEnabled{false},
    m_pushButtonProtoOpenvpnContDefaultChecked{false},
    m_pushButtonProtoSsOpenvpnContDefaultChecked{false},
    m_pushButtonProtoCloakOpenvpnContDefaultChecked{false},
    m_pushButtonProtoWireguardContDefaultChecked{false},
    m_pushButtonProtoOpenvpnContDefaultVisible{true},
    m_pushButtonProtoSsOpenvpnContDefaultVisible{false},
    m_pushButtonProtoCloakOpenvpnContDefaultVisible{false},
    m_pushButtonProtoWireguardContDefaultVisible{false},
    m_pushButtonProtoOpenvpnContShareVisible{false},
    m_pushButtonProtoSsOpenvpnContShareVisible{false},
    m_pushButtonProtoCloakOpenvpnContShareVisible{false},
    m_pushButtonProtoWireguardContShareVisible{false},
    m_frameOpenvpnSettingsVisible{true},
    m_frameOpenvpnSsSettingsVisible{true},
    m_frameOpenvpnSsCloakSettingsVisible{true},
    m_progressBarProtocolsContainerReinstallVisible{false},
    m_labelSpeedReceivedText{tr("0 Mbps")},
    m_labelSpeedSentText{tr("0 Mbps")},
    m_labelStateText{},
    m_pushButtonConnectEnabled{false},
    m_widgetVpnModeEnabled{false},
    m_labelErrorText{tr("Error text")},
    m_dialogConnectErrorText{},
    m_pageNewServerConfiguringEnabled{true},
    m_labelNewServerConfiguringWaitInfoVisible{true},
    m_labelNewServerConfiguringWaitInfoText{tr("Please wait, configuring process may take up to 5 minutes")},
    m_progressBarNewServerConfiguringVisible{true},
    m_progressBarNewServerConfiguringMaximium{100},
    m_progressBarNewServerConfiguringTextVisible{true},
    m_progressBarNewServerConfiguringText{tr("Configuring...")},
    m_pageServerProtocolsEnabled{true},
    m_progressBarProtocolsContainerReinstallValue{0},
    m_progressBarProtocolsContainerReinstallMaximium{100},


    m_vpnConnection(nullptr)
{
    m_vpnConnection = new VpnConnection(this);

    m_appSettingsLogic = new AppSettingsLogic(this);
    m_generalSettingsLogic = new GeneralSettingsLogic(this);
    m_networkSettingsLogic = new NetworkSettingsLogic(this);
    m_newServerProtocolsLogic = new NewServerProtocolsLogic(this);
    m_protocolSettingsLogic = new ProtocolSettingsLogic(this);
    m_serverListLogic = new ServerListLogic(this);
    m_serverSettingsLogic = new ServerSettingsLogic(this);
    m_serverVpnProtocolsLogic = new ServerVpnProtocolsLogic(this);
    m_shareConnectionLogic = new ShareConnectionLogic(this);
    m_sitesLogic = new SitesLogic(this);
    m_startPageLogic = new StartPageLogic(this);
    m_vpnLogic = new VpnLogic(this);
    m_wizardLogic = new WizardLogic(this);

    m_openVpnLogic = new OpenVpnLogic(this);
    m_shadowSocksLogic = new ShadowSocksLogic(this);
    m_cloakLogic = new CloakLogic(this);

    connect(m_vpnConnection, SIGNAL(bytesChanged(quint64, quint64)), this, SLOT(onBytesChanged(quint64, quint64)));
    connect(m_vpnConnection, SIGNAL(connectionStateChanged(VpnProtocol::ConnectionState)), this, SLOT(onConnectionStateChanged(VpnProtocol::ConnectionState)));
    connect(m_vpnConnection, SIGNAL(vpnProtocolError(amnezia::ErrorCode)), this, SLOT(onVpnProtocolError(amnezia::ErrorCode)));
}

void UiLogic::initalizeUiLogic()
{
    setFrameWireguardSettingsVisible(false);
    setFrameWireguardVisible(false);
    setFrameNewServerSettingsParentWireguardVisible(false);

    setupTray();
    setupNewServerConnections();
    setupProtocolsPageConnections();

    setLabelErrorText("");


    //    if (QOperatingSystemVersion::current() <= QOperatingSystemVersion::Windows7) {
    //        needToHideCustomTitlebar = true;
    //    }

    //#if defined Q_OS_MAC
    //    fixWidget(this);
    //    needToHideCustomTitlebar = true;
    //#endif

    //    if (needToHideCustomTitlebar) {
    //        ui->widget_tittlebar->hide();
    //        resize(width(), 640);
    //        ui->stackedWidget_main->move(0,0);
    //    }

    // Post initialization
    goToPage(Page::Start, true, false);

    if (m_settings.defaultServerIndex() >= 0 && m_settings.serversCount() > 0) {
        goToPage(Page::Vpn, true, false);
    }

    //    //ui->pushButton_general_settings_exit->hide();


    qInfo().noquote() << QString("Started %1 version %2").arg(APPLICATION_NAME).arg(APP_VERSION);
    qInfo().noquote() << QString("%1 (%2)").arg(QSysInfo::prettyProductName()).arg(QSysInfo::currentCpuArchitecture());



    onConnectionStateChanged(VpnProtocol::Disconnected);

    if (m_settings.isAutoConnect() && m_settings.defaultServerIndex() >= 0) {
        QTimer::singleShot(1000, this, [this](){
            setPushButtonConnectEnabled(false);
            onConnect();
        });
    }

    //    m_ipAddressValidator.setRegExp(Utils::ipAddressRegExp());
    //    m_ipAddressPortValidator.setRegExp(Utils::ipAddressPortRegExp());
    //    m_ipNetwok24Validator.setRegExp(Utils::ipNetwork24RegExp());
    //    m_ipPortValidator.setRegExp(Utils::ipPortRegExp());

    //    ui->lineEdit_new_server_ip->setValidator(&m_ipAddressPortValidator);
    //    ui->lineEdit_network_settings_dns1->setValidator(&m_ipAddressValidator);
    //    ui->lineEdit_network_settings_dns2->setValidator(&m_ipAddressValidator);

    //    ui->lineEdit_proto_openvpn_subnet->setValidator(&m_ipNetwok24Validator);

    //    ui->lineEdit_proto_openvpn_port->setValidator(&m_ipPortValidator);
    //    ui->lineEdit_proto_shadowsocks_port->setValidator(&m_ipPortValidator);
    //    ui->lineEdit_proto_cloak_port->setValidator(&m_ipPortValidator);




}

bool UiLogic::getFrameWireguardSettingsVisible() const
{
    return m_frameWireguardSettingsVisible;
}

void UiLogic::setFrameWireguardSettingsVisible(bool frameWireguardSettingsVisible)
{
    if (m_frameWireguardSettingsVisible != frameWireguardSettingsVisible) {
        m_frameWireguardSettingsVisible = frameWireguardSettingsVisible;
        emit frameWireguardSettingsVisibleChanged();
    }
}

bool UiLogic::getFrameWireguardVisible() const
{
    return m_frameWireguardVisible;
}

void UiLogic::setFrameWireguardVisible(bool frameWireguardVisible)
{
    if (m_frameWireguardVisible != frameWireguardVisible) {
        m_frameWireguardVisible = frameWireguardVisible;
        emit frameWireguardVisibleChanged();
    }
}

bool UiLogic::getFrameNewServerSettingsParentWireguardVisible() const
{
    return m_frameNewServerSettingsParentWireguardVisible;
}

void UiLogic::setFrameNewServerSettingsParentWireguardVisible(bool frameNewServerSettingsParentWireguardVisible)
{
    if (m_frameNewServerSettingsParentWireguardVisible != frameNewServerSettingsParentWireguardVisible) {
        m_frameNewServerSettingsParentWireguardVisible = frameNewServerSettingsParentWireguardVisible;
        emit frameNewServerSettingsParentWireguardVisibleChanged();
    }
}



void UiLogic::updateNewServerProtocolsPage()
{
    setPushButtonNewServerSettingsCloakChecked(true);
    setPushButtonNewServerSettingsCloakChecked(false);
    setPushButtonNewServerSettingsSsChecked(true);
    setPushButtonNewServerSettingsSsChecked(false);
    setLineEditNewServerCloakPortText(amnezia::protocols::cloak::defaultPort);
    setLineEditNewServerCloakSiteText(amnezia::protocols::cloak::defaultRedirSite);
    setLineEditNewServerSsPortText(amnezia::protocols::shadowsocks::defaultPort);
    setComboBoxNewServerSsCipherText(amnezia::protocols::shadowsocks::defaultCipher);
    setLineEditNewServerOpenvpnPortText(amnezia::protocols::openvpn::defaultPort);
    setComboBoxNewServerOpenvpnProtoText(amnezia::protocols::openvpn::defaultTransportProto);
}



QString UiLogic::getComboBoxNewServerOpenvpnProtoText() const
{
    return m_comboBoxNewServerOpenvpnProtoText;
}

void UiLogic::setComboBoxNewServerOpenvpnProtoText(const QString &comboBoxNewServerOpenvpnProtoText)
{
    if (m_comboBoxNewServerOpenvpnProtoText != comboBoxNewServerOpenvpnProtoText) {
        m_comboBoxNewServerOpenvpnProtoText = comboBoxNewServerOpenvpnProtoText;
        emit comboBoxNewServerOpenvpnProtoTextChanged();
    }
}

QString UiLogic::getLineEditNewServerCloakSiteText() const
{
    return m_lineEditNewServerCloakSiteText;
}

void UiLogic::setLineEditNewServerCloakSiteText(const QString &lineEditNewServerCloakSiteText)
{
    if (m_lineEditNewServerCloakSiteText != lineEditNewServerCloakSiteText) {
        m_lineEditNewServerCloakSiteText = lineEditNewServerCloakSiteText;
        emit lineEditNewServerCloakSiteTextChanged();
    }
}

QString UiLogic::getLineEditNewServerSsPortText() const
{
    return m_lineEditNewServerSsPortText;
}

void UiLogic::setLineEditNewServerSsPortText(const QString &lineEditNewServerSsPortText)
{
    if (m_lineEditNewServerSsPortText != lineEditNewServerSsPortText) {
        m_lineEditNewServerSsPortText = lineEditNewServerSsPortText;
        emit lineEditNewServerSsPortTextChanged();
    }
}

QString UiLogic::getComboBoxNewServerSsCipherText() const
{
    return m_comboBoxNewServerSsCipherText;
}

void UiLogic::setComboBoxNewServerSsCipherText(const QString &comboBoxNewServerSsCipherText)
{
    if (m_comboBoxNewServerSsCipherText != comboBoxNewServerSsCipherText) {
        m_comboBoxNewServerSsCipherText = comboBoxNewServerSsCipherText;
        emit comboBoxNewServerSsCipherTextChanged();
    }
}

QString UiLogic::getlineEditNewServerOpenvpnPortText() const
{
    return m_lineEditNewServerOpenvpnPortText;
}

void UiLogic::setLineEditNewServerOpenvpnPortText(const QString &lineEditNewServerOpenvpnPortText)
{
    if (m_lineEditNewServerOpenvpnPortText != lineEditNewServerOpenvpnPortText) {
        m_lineEditNewServerOpenvpnPortText = lineEditNewServerOpenvpnPortText;
        emit lineEditNewServerOpenvpnPortTextChanged();
    }
}

bool UiLogic::getPushButtonNewServerSettingsSsChecked() const
{
    return m_pushButtonNewServerSettingsSsChecked;
}

void UiLogic::setPushButtonNewServerSettingsSsChecked(bool pushButtonNewServerSettingsSsChecked)
{
    if (m_pushButtonNewServerSettingsSsChecked != pushButtonNewServerSettingsSsChecked) {
        m_pushButtonNewServerSettingsSsChecked = pushButtonNewServerSettingsSsChecked;
        emit pushButtonNewServerSettingsSsCheckedChanged();
    }
}

bool UiLogic::getPushButtonNewServerSettingsOpenvpnChecked() const
{
    return m_pushButtonNewServerSettingsOpenvpnChecked;
}

void UiLogic::setPushButtonNewServerSettingsOpenvpnChecked(bool pushButtonNewServerSettingsOpenvpnChecked)
{
    if (m_pushButtonNewServerSettingsOpenvpnChecked != pushButtonNewServerSettingsOpenvpnChecked) {
        m_pushButtonNewServerSettingsOpenvpnChecked = pushButtonNewServerSettingsOpenvpnChecked;
        emit pushButtonNewServerSettingsOpenvpnCheckedChanged();
    }
}

QString UiLogic::getLineEditNewServerCloakPortText() const
{
    return m_lineEditNewServerCloakPortText;
}

void UiLogic::setLineEditNewServerCloakPortText(const QString &lineEditNewServerCloakPortText)
{
    if (m_lineEditNewServerCloakPortText != lineEditNewServerCloakPortText) {
        m_lineEditNewServerCloakPortText = lineEditNewServerCloakPortText;
        emit lineEditNewServerCloakPortTextChanged();
    }
}

bool UiLogic::getPushButtonNewServerSettingsCloakChecked() const
{
    return m_pushButtonNewServerSettingsCloakChecked;
}

void UiLogic::setPushButtonNewServerSettingsCloakChecked(bool pushButtonNewServerSettingsCloakChecked)
{
    if (m_pushButtonNewServerSettingsCloakChecked != pushButtonNewServerSettingsCloakChecked) {
        m_pushButtonNewServerSettingsCloakChecked = pushButtonNewServerSettingsCloakChecked;
        emit pushButtonNewServerSettingsCloakCheckedChanged();
    }
}

double UiLogic::getProgressBarNewServerConfiguringValue() const
{
    return m_progressBarNewServerConfiguringValue;
}

void UiLogic::setProgressBarNewServerConfiguringValue(double progressBarNewServerConfiguringValue)
{
    if (m_progressBarNewServerConfiguringValue != progressBarNewServerConfiguringValue) {
        m_progressBarNewServerConfiguringValue = progressBarNewServerConfiguringValue;
        emit progressBarNewServerConfiguringValueChanged();
    }
}







bool UiLogic::getRadioButtonVpnModeAllSitesChecked() const
{
    return m_radioButtonVpnModeAllSitesChecked;
}

bool UiLogic::getRadioButtonVpnModeForwardSitesChecked() const
{
    return m_radioButtonVpnModeForwardSitesChecked;
}

bool UiLogic::getRadioButtonVpnModeExceptSitesChecked() const
{
    return m_radioButtonVpnModeExceptSitesChecked;
}

void UiLogic::setRadioButtonVpnModeAllSitesChecked(bool radioButtonVpnModeAllSitesChecked)
{
    if (m_radioButtonVpnModeAllSitesChecked != radioButtonVpnModeAllSitesChecked) {
        m_radioButtonVpnModeAllSitesChecked = radioButtonVpnModeAllSitesChecked;
        emit radioButtonVpnModeAllSitesCheckedChanged();
    }
}

void UiLogic::setRadioButtonVpnModeForwardSitesChecked(bool radioButtonVpnModeForwardSitesChecked)
{
    if (m_radioButtonVpnModeForwardSitesChecked != radioButtonVpnModeForwardSitesChecked) {
        m_radioButtonVpnModeForwardSitesChecked = radioButtonVpnModeForwardSitesChecked;
        emit radioButtonVpnModeForwardSitesCheckedChanged();
    }
}

void UiLogic::setRadioButtonVpnModeExceptSitesChecked(bool radioButtonVpnModeExceptSitesChecked)
{
    if (m_radioButtonVpnModeExceptSitesChecked != radioButtonVpnModeExceptSitesChecked) {
        m_radioButtonVpnModeExceptSitesChecked = radioButtonVpnModeExceptSitesChecked;
        emit radioButtonVpnModeExceptSitesCheckedChanged();
    }
}









int UiLogic::getCurrentPageValue() const
{
    return m_currentPageValue;
}

void UiLogic::setCurrentPageValue(int currentPageValue)
{
    if (m_currentPageValue != currentPageValue) {
        m_currentPageValue = currentPageValue;
        emit currentPageValueChanged();
    }
}

QString UiLogic::getTrayIconUrl() const
{
    return m_trayIconUrl;
}

void UiLogic::setTrayIconUrl(const QString &trayIconUrl)
{
    if (m_trayIconUrl != trayIconUrl) {
        m_trayIconUrl = trayIconUrl;
        emit trayIconUrlChanged();
    }
}

bool UiLogic::getTrayActionDisconnectEnabled() const
{
    return m_trayActionDisconnectEnabled;
}

void UiLogic::setTrayActionDisconnectEnabled(bool trayActionDisconnectEnabled)
{
    if (m_trayActionDisconnectEnabled != trayActionDisconnectEnabled) {
        m_trayActionDisconnectEnabled = trayActionDisconnectEnabled;
        emit trayActionDisconnectEnabledChanged();
    }
}

bool UiLogic::getTrayActionConnectEnabled() const
{
    return m_trayActionConnectEnabled;
}

void UiLogic::setTrayActionConnectEnabled(bool trayActionConnectEnabled)
{
    if (m_trayActionConnectEnabled != trayActionConnectEnabled) {
        m_trayActionConnectEnabled = trayActionConnectEnabled;
        emit trayActionConnectEnabledChanged();
    }
}

bool UiLogic::getCheckBoxNewServerCloakChecked() const
{
    return m_checkBoxNewServerCloakChecked;
}

void UiLogic::setCheckBoxNewServerCloakChecked(bool checkBoxNewServerCloakChecked)
{
    if (m_checkBoxNewServerCloakChecked != checkBoxNewServerCloakChecked) {
        m_checkBoxNewServerCloakChecked = checkBoxNewServerCloakChecked;
        emit checkBoxNewServerCloakCheckedChanged();
    }
}

bool UiLogic::getCheckBoxNewServerSsChecked() const
{
    return m_checkBoxNewServerSsChecked;
}

void UiLogic::setCheckBoxNewServerSsChecked(bool checkBoxNewServerSsChecked)
{
    if (m_checkBoxNewServerSsChecked != checkBoxNewServerSsChecked) {
        m_checkBoxNewServerSsChecked = checkBoxNewServerSsChecked;
        emit checkBoxNewServerSsCheckedChanged();
    }
}

bool UiLogic::getCheckBoxNewServerOpenvpnChecked() const
{
    return m_checkBoxNewServerOpenvpnChecked;
}

void UiLogic::setCheckBoxNewServerOpenvpnChecked(bool checkBoxNewServerOpenvpnChecked)
{
    if (m_checkBoxNewServerOpenvpnChecked != checkBoxNewServerOpenvpnChecked) {
        m_checkBoxNewServerOpenvpnChecked = checkBoxNewServerOpenvpnChecked;
        emit checkBoxNewServerOpenvpnCheckedChanged();
    }
}









bool UiLogic::getPushButtonConnectChecked() const
{
    return m_pushButtonConnectChecked;
}

void UiLogic::setPushButtonConnectChecked(bool pushButtonConnectChecked)
{
    if (m_pushButtonConnectChecked != pushButtonConnectChecked) {
        m_pushButtonConnectChecked = pushButtonConnectChecked;
        emit pushButtonConnectCheckedChanged();
    }
}

bool UiLogic::getPushButtonVpnAddSiteEnabled() const
{
    return m_pushButtonVpnAddSiteEnabled;
}

void UiLogic::setPushButtonVpnAddSiteEnabled(bool pushButtonVpnAddSiteEnabled)
{
    if (m_pushButtonVpnAddSiteEnabled != pushButtonVpnAddSiteEnabled) {
        m_pushButtonVpnAddSiteEnabled = pushButtonVpnAddSiteEnabled;
        emit pushButtonVpnAddSiteEnabledChanged();
    }
}








bool UiLogic::getPushButtonProtoOpenvpnContInstallChecked() const
{
    return m_pushButtonProtoOpenvpnContInstallChecked;
}

void UiLogic::setPushButtonProtoOpenvpnContInstallChecked(bool pushButtonProtoOpenvpnContInstallChecked)
{
    if (m_pushButtonProtoOpenvpnContInstallChecked != pushButtonProtoOpenvpnContInstallChecked) {
        m_pushButtonProtoOpenvpnContInstallChecked = pushButtonProtoOpenvpnContInstallChecked;
        emit pushButtonProtoOpenvpnContInstallCheckedChanged();
    }
}

bool UiLogic::getPushButtonProtoSsOpenvpnContInstallChecked() const
{
    return m_pushButtonProtoSsOpenvpnContInstallChecked;
}

void UiLogic::setPushButtonProtoSsOpenvpnContInstallChecked(bool pushButtonProtoSsOpenvpnContInstallChecked)
{
    if (m_pushButtonProtoSsOpenvpnContInstallChecked != pushButtonProtoSsOpenvpnContInstallChecked) {
        m_pushButtonProtoSsOpenvpnContInstallChecked = pushButtonProtoSsOpenvpnContInstallChecked;
        emit pushButtonProtoSsOpenvpnContInstallCheckedChanged();
    }
}

bool UiLogic::getPushButtonProtoCloakOpenvpnContInstallChecked() const
{
    return m_pushButtonProtoCloakOpenvpnContInstallChecked;
}

void UiLogic::setPushButtonProtoCloakOpenvpnContInstallChecked(bool pushButtonProtoCloakOpenvpnContInstallChecked)
{
    if (m_pushButtonProtoCloakOpenvpnContInstallChecked != pushButtonProtoCloakOpenvpnContInstallChecked) {
        m_pushButtonProtoCloakOpenvpnContInstallChecked = pushButtonProtoCloakOpenvpnContInstallChecked;
        emit pushButtonProtoCloakOpenvpnContInstallCheckedChanged();
    }
}

bool UiLogic::getPushButtonProtoWireguardContInstallChecked() const
{
    return m_pushButtonProtoWireguardContInstallChecked;
}

void UiLogic::setPushButtonProtoWireguardContInstallChecked(bool pushButtonProtoWireguardContInstallChecked)
{
    if (m_pushButtonProtoWireguardContInstallChecked != pushButtonProtoWireguardContInstallChecked) {
        m_pushButtonProtoWireguardContInstallChecked = pushButtonProtoWireguardContInstallChecked;
        emit pushButtonProtoWireguardContInstallCheckedChanged();
    }
}

bool UiLogic::getPushButtonProtoOpenvpnContInstallEnabled() const
{
    return m_pushButtonProtoOpenvpnContInstallEnabled;
}

void UiLogic::setPushButtonProtoOpenvpnContInstallEnabled(bool pushButtonProtoOpenvpnContInstallEnabled)
{
    if (m_pushButtonProtoOpenvpnContInstallEnabled != pushButtonProtoOpenvpnContInstallEnabled) {
        m_pushButtonProtoOpenvpnContInstallEnabled = pushButtonProtoOpenvpnContInstallEnabled;
        emit pushButtonProtoOpenvpnContInstallEnabledChanged();
    }
}

bool UiLogic::getPushButtonProtoSsOpenvpnContInstallEnabled() const
{
    return m_pushButtonProtoSsOpenvpnContInstallEnabled;
}

void UiLogic::setPushButtonProtoSsOpenvpnContInstallEnabled(bool pushButtonProtoSsOpenvpnContInstallEnabled)
{
    if (m_pushButtonProtoSsOpenvpnContInstallEnabled != pushButtonProtoSsOpenvpnContInstallEnabled) {
        m_pushButtonProtoSsOpenvpnContInstallEnabled = pushButtonProtoSsOpenvpnContInstallEnabled;
        emit pushButtonProtoSsOpenvpnContInstallEnabledChanged();
    }
}

bool UiLogic::getPushButtonProtoCloakOpenvpnContInstallEnabled() const
{
    return m_pushButtonProtoCloakOpenvpnContInstallEnabled;
}

void UiLogic::setPushButtonProtoCloakOpenvpnContInstallEnabled(bool pushButtonProtoCloakOpenvpnContInstallEnabled)
{
    if (m_pushButtonProtoCloakOpenvpnContInstallEnabled != pushButtonProtoCloakOpenvpnContInstallEnabled) {
        m_pushButtonProtoCloakOpenvpnContInstallEnabled = pushButtonProtoCloakOpenvpnContInstallEnabled;
        emit pushButtonProtoCloakOpenvpnContInstallEnabledChanged();
    }
}

bool UiLogic::getPushButtonProtoWireguardContInstallEnabled() const
{
    return m_pushButtonProtoWireguardContInstallEnabled;
}

void UiLogic::setPushButtonProtoWireguardContInstallEnabled(bool pushButtonProtoWireguardContInstallEnabled)
{
    if (m_pushButtonProtoWireguardContInstallEnabled != pushButtonProtoWireguardContInstallEnabled) {
        m_pushButtonProtoWireguardContInstallEnabled = pushButtonProtoWireguardContInstallEnabled;
        emit pushButtonProtoWireguardContInstallEnabledChanged();
    }
}

bool UiLogic::getPushButtonProtoOpenvpnContDefaultChecked() const
{
    return m_pushButtonProtoOpenvpnContDefaultChecked;
}

void UiLogic::setPushButtonProtoOpenvpnContDefaultChecked(bool pushButtonProtoOpenvpnContDefaultChecked)
{
    if (m_pushButtonProtoOpenvpnContDefaultChecked != pushButtonProtoOpenvpnContDefaultChecked) {
        m_pushButtonProtoOpenvpnContDefaultChecked = pushButtonProtoOpenvpnContDefaultChecked;
        emit pushButtonProtoOpenvpnContDefaultCheckedChanged();
    }
}

bool UiLogic::getPushButtonProtoSsOpenvpnContDefaultChecked() const
{
    return m_pushButtonProtoSsOpenvpnContDefaultChecked;
}

void UiLogic::setPushButtonProtoSsOpenvpnContDefaultChecked(bool pushButtonProtoSsOpenvpnContDefaultChecked)
{
    if (m_pushButtonProtoSsOpenvpnContDefaultChecked != pushButtonProtoSsOpenvpnContDefaultChecked) {
        m_pushButtonProtoSsOpenvpnContDefaultChecked = pushButtonProtoSsOpenvpnContDefaultChecked;
        emit pushButtonProtoSsOpenvpnContDefaultCheckedChanged();
    }
}

bool UiLogic::getPushButtonProtoCloakOpenvpnContDefaultChecked() const
{
    return m_pushButtonProtoCloakOpenvpnContDefaultChecked;
}

void UiLogic::setPushButtonProtoCloakOpenvpnContDefaultChecked(bool pushButtonProtoCloakOpenvpnContDefaultChecked)
{
    if (m_pushButtonProtoCloakOpenvpnContDefaultChecked != pushButtonProtoCloakOpenvpnContDefaultChecked) {
        m_pushButtonProtoCloakOpenvpnContDefaultChecked = pushButtonProtoCloakOpenvpnContDefaultChecked;
        emit pushButtonProtoCloakOpenvpnContDefaultCheckedChanged();
    }
}

bool UiLogic::getPushButtonProtoWireguardContDefaultChecked() const
{
    return m_pushButtonProtoWireguardContDefaultChecked;
}

void UiLogic::setPushButtonProtoWireguardContDefaultChecked(bool pushButtonProtoWireguardContDefaultChecked)
{
    if (m_pushButtonProtoWireguardContDefaultChecked != pushButtonProtoWireguardContDefaultChecked) {
        m_pushButtonProtoWireguardContDefaultChecked = pushButtonProtoWireguardContDefaultChecked;
        emit pushButtonProtoWireguardContDefaultCheckedChanged();
    }
}

bool UiLogic::getPushButtonProtoOpenvpnContDefaultVisible() const
{
    return m_pushButtonProtoOpenvpnContDefaultVisible;
}

void UiLogic::setPushButtonProtoOpenvpnContDefaultVisible(bool pushButtonProtoOpenvpnContDefaultVisible)
{
    if (m_pushButtonProtoOpenvpnContDefaultVisible != pushButtonProtoOpenvpnContDefaultVisible) {
        m_pushButtonProtoOpenvpnContDefaultVisible = pushButtonProtoOpenvpnContDefaultVisible;
        emit pushButtonProtoOpenvpnContDefaultVisibleChanged();
    }
}

bool UiLogic::getPushButtonProtoSsOpenvpnContDefaultVisible() const
{
    return m_pushButtonProtoSsOpenvpnContDefaultVisible;
}

void UiLogic::setPushButtonProtoSsOpenvpnContDefaultVisible(bool pushButtonProtoSsOpenvpnContDefaultVisible)
{
    if (m_pushButtonProtoSsOpenvpnContDefaultVisible != pushButtonProtoSsOpenvpnContDefaultVisible) {
        m_pushButtonProtoSsOpenvpnContDefaultVisible = pushButtonProtoSsOpenvpnContDefaultVisible;
        emit pushButtonProtoSsOpenvpnContDefaultVisibleChanged();
    }
}

bool UiLogic::getPushButtonProtoCloakOpenvpnContDefaultVisible() const
{
    return m_pushButtonProtoCloakOpenvpnContDefaultVisible;
}

void UiLogic::setPushButtonProtoCloakOpenvpnContDefaultVisible(bool pushButtonProtoCloakOpenvpnContDefaultVisible)
{
    if (m_pushButtonProtoCloakOpenvpnContDefaultVisible != pushButtonProtoCloakOpenvpnContDefaultVisible) {
        m_pushButtonProtoCloakOpenvpnContDefaultVisible = pushButtonProtoCloakOpenvpnContDefaultVisible;
        emit pushButtonProtoCloakOpenvpnContDefaultVisibleChanged();
    }
}

bool UiLogic::getPushButtonProtoWireguardContDefaultVisible() const
{
    return m_pushButtonProtoWireguardContDefaultVisible;
}

void UiLogic::setPushButtonProtoWireguardContDefaultVisible(bool pushButtonProtoWireguardContDefaultVisible)
{
    if (m_pushButtonProtoWireguardContDefaultVisible != pushButtonProtoWireguardContDefaultVisible) {
        m_pushButtonProtoWireguardContDefaultVisible = pushButtonProtoWireguardContDefaultVisible;
        emit pushButtonProtoWireguardContDefaultVisibleChanged();
    }
}

bool UiLogic::getPushButtonProtoOpenvpnContShareVisible() const
{
    return m_pushButtonProtoOpenvpnContShareVisible;
}

void UiLogic::setPushButtonProtoOpenvpnContShareVisible(bool pushButtonProtoOpenvpnContShareVisible)
{
    if (m_pushButtonProtoOpenvpnContShareVisible != pushButtonProtoOpenvpnContShareVisible) {
        m_pushButtonProtoOpenvpnContShareVisible = pushButtonProtoOpenvpnContShareVisible;
        emit pushButtonProtoOpenvpnContShareVisibleChanged();
    }
}

bool UiLogic::getPushButtonProtoSsOpenvpnContShareVisible() const
{
    return m_pushButtonProtoSsOpenvpnContShareVisible;
}

void UiLogic::setPushButtonProtoSsOpenvpnContShareVisible(bool pushButtonProtoSsOpenvpnContShareVisible)
{
    if (m_pushButtonProtoSsOpenvpnContShareVisible != pushButtonProtoSsOpenvpnContShareVisible) {
        m_pushButtonProtoSsOpenvpnContShareVisible = pushButtonProtoSsOpenvpnContShareVisible;
        emit pushButtonProtoSsOpenvpnContShareVisibleChanged();
    }
}

bool UiLogic::getPushButtonProtoCloakOpenvpnContShareVisible() const
{
    return m_pushButtonProtoCloakOpenvpnContShareVisible;
}

void UiLogic::setPushButtonProtoCloakOpenvpnContShareVisible(bool pushButtonProtoCloakOpenvpnContShareVisible)
{
    if (m_pushButtonProtoCloakOpenvpnContShareVisible != pushButtonProtoCloakOpenvpnContShareVisible) {
        m_pushButtonProtoCloakOpenvpnContShareVisible = pushButtonProtoCloakOpenvpnContShareVisible;
        emit pushButtonProtoCloakOpenvpnContShareVisibleChanged();
    }
}

bool UiLogic::getPushButtonProtoWireguardContShareVisible() const
{
    return m_pushButtonProtoWireguardContShareVisible;
}

void UiLogic::setPushButtonProtoWireguardContShareVisible(bool pushButtonProtoWireguardContShareVisible)
{
    if (m_pushButtonProtoWireguardContShareVisible != pushButtonProtoWireguardContShareVisible) {
        m_pushButtonProtoWireguardContShareVisible = pushButtonProtoWireguardContShareVisible;
        emit pushButtonProtoWireguardContShareVisibleChanged();
    }
}

bool UiLogic::getFrameOpenvpnSettingsVisible() const
{
    return m_frameOpenvpnSettingsVisible;
}

void UiLogic::setFrameOpenvpnSettingsVisible(bool frameOpenvpnSettingsVisible)
{
    if (m_frameOpenvpnSettingsVisible != frameOpenvpnSettingsVisible) {
        m_frameOpenvpnSettingsVisible = frameOpenvpnSettingsVisible;
        emit frameOpenvpnSettingsVisibleChanged();
    }
}

bool UiLogic::getFrameOpenvpnSsSettingsVisible() const
{
    return m_frameOpenvpnSsSettingsVisible;
}

void UiLogic::setFrameOpenvpnSsSettingsVisible(bool frameOpenvpnSsSettingsVisible)
{
    if (m_frameOpenvpnSsSettingsVisible != frameOpenvpnSsSettingsVisible) {
        m_frameOpenvpnSsSettingsVisible = frameOpenvpnSsSettingsVisible;
        emit frameOpenvpnSsSettingsVisibleChanged();
    }
}

bool UiLogic::getFrameOpenvpnSsCloakSettingsVisible() const
{
    return m_frameOpenvpnSsCloakSettingsVisible;
}

void UiLogic::setFrameOpenvpnSsCloakSettingsVisible(bool frameOpenvpnSsCloakSettingsVisible)
{
    if (m_frameOpenvpnSsCloakSettingsVisible != frameOpenvpnSsCloakSettingsVisible) {
        m_frameOpenvpnSsCloakSettingsVisible = frameOpenvpnSsCloakSettingsVisible;
        emit frameOpenvpnSsCloakSettingsVisibleChanged();
    }
}

bool UiLogic::getProgressBarProtocolsContainerReinstallVisible() const
{
    return m_progressBarProtocolsContainerReinstallVisible;
}

void UiLogic::setProgressBarProtocolsContainerReinstallVisible(bool progressBarProtocolsContainerReinstallVisible)
{
    if (m_progressBarProtocolsContainerReinstallVisible != progressBarProtocolsContainerReinstallVisible) {
        m_progressBarProtocolsContainerReinstallVisible = progressBarProtocolsContainerReinstallVisible;
        emit progressBarProtocolsContainerReinstallVisibleChanged();
    }
}

QString UiLogic::getLabelSpeedReceivedText() const
{
    return m_labelSpeedReceivedText;
}

void UiLogic::setLabelSpeedReceivedText(const QString &labelSpeedReceivedText)
{
    if (m_labelSpeedReceivedText != labelSpeedReceivedText) {
        m_labelSpeedReceivedText = labelSpeedReceivedText;
        emit labelSpeedReceivedTextChanged();
    }
}

QString UiLogic::getLabelSpeedSentText() const
{
    return m_labelSpeedSentText;
}

void UiLogic::setLabelSpeedSentText(const QString &labelSpeedSentText)
{
    if (m_labelSpeedSentText != labelSpeedSentText) {
        m_labelSpeedSentText = labelSpeedSentText;
        emit labelSpeedSentTextChanged();
    }
}

QString UiLogic::getLabelStateText() const
{
    return m_labelStateText;
}

void UiLogic::setLabelStateText(const QString &labelStateText)
{
    if (m_labelStateText != labelStateText) {
        m_labelStateText = labelStateText;
        emit labelStateTextChanged();
    }
}

bool UiLogic::getPushButtonConnectEnabled() const
{
    return m_pushButtonConnectEnabled;
}

void UiLogic::setPushButtonConnectEnabled(bool pushButtonConnectEnabled)
{
    if (m_pushButtonConnectEnabled != pushButtonConnectEnabled) {
        m_pushButtonConnectEnabled = pushButtonConnectEnabled;
        emit pushButtonConnectEnabledChanged();
    }
}

bool UiLogic::getWidgetVpnModeEnabled() const
{
    return m_widgetVpnModeEnabled;
}

void UiLogic::setWidgetVpnModeEnabled(bool widgetVpnModeEnabled)
{
    if (m_widgetVpnModeEnabled != widgetVpnModeEnabled) {
        m_widgetVpnModeEnabled = widgetVpnModeEnabled;
        emit widgetVpnModeEnabledChanged();
    }
}

QString UiLogic::getLabelErrorText() const
{
    return m_labelErrorText;
}

void UiLogic::setLabelErrorText(const QString &labelErrorText)
{
    if (m_labelErrorText != labelErrorText) {
        m_labelErrorText = labelErrorText;
        emit labelErrorTextChanged();
    }
}


QString UiLogic::getDialogConnectErrorText() const
{
    return m_dialogConnectErrorText;
}

void UiLogic::setDialogConnectErrorText(const QString &dialogConnectErrorText)
{
    if (m_dialogConnectErrorText != dialogConnectErrorText) {
        m_dialogConnectErrorText = dialogConnectErrorText;
        emit dialogConnectErrorTextChanged();
    }
}






bool UiLogic::getPageNewServerConfiguringEnabled() const
{
    return m_pageNewServerConfiguringEnabled;
}

void UiLogic::setPageNewServerConfiguringEnabled(bool pageNewServerConfiguringEnabled)
{
    if (m_pageNewServerConfiguringEnabled != pageNewServerConfiguringEnabled) {
        m_pageNewServerConfiguringEnabled = pageNewServerConfiguringEnabled;
        emit pageNewServerConfiguringEnabledChanged();
    }
}

bool UiLogic::getLabelNewServerConfiguringWaitInfoVisible() const
{
    return m_labelNewServerConfiguringWaitInfoVisible;
}

void UiLogic::setLabelNewServerConfiguringWaitInfoVisible(bool labelNewServerConfiguringWaitInfoVisible)
{
    if (m_labelNewServerConfiguringWaitInfoVisible != labelNewServerConfiguringWaitInfoVisible) {
        m_labelNewServerConfiguringWaitInfoVisible = labelNewServerConfiguringWaitInfoVisible;
        emit labelNewServerConfiguringWaitInfoVisibleChanged();
    }
}

QString UiLogic::getLabelNewServerConfiguringWaitInfoText() const
{
    return m_labelNewServerConfiguringWaitInfoText;
}

void UiLogic::setLabelNewServerConfiguringWaitInfoText(const QString &labelNewServerConfiguringWaitInfoText)
{
    if (m_labelNewServerConfiguringWaitInfoText != labelNewServerConfiguringWaitInfoText) {
        m_labelNewServerConfiguringWaitInfoText = labelNewServerConfiguringWaitInfoText;
        emit labelNewServerConfiguringWaitInfoTextChanged();
    }
}

bool UiLogic::getProgressBarNewServerConfiguringVisible() const
{
    return m_progressBarNewServerConfiguringVisible;
}

void UiLogic::setProgressBarNewServerConfiguringVisible(bool progressBarNewServerConfiguringVisible)
{
    if (m_progressBarNewServerConfiguringVisible != progressBarNewServerConfiguringVisible) {
        m_progressBarNewServerConfiguringVisible = progressBarNewServerConfiguringVisible;
        emit progressBarNewServerConfiguringVisibleChanged();
    }
}

int UiLogic::getProgressBarNewServerConfiguringMaximium() const
{
    return m_progressBarNewServerConfiguringMaximium;
}

void UiLogic::setProgressBarNewServerConfiguringMaximium(int progressBarNewServerConfiguringMaximium)
{
    if (m_progressBarNewServerConfiguringMaximium != progressBarNewServerConfiguringMaximium) {
        m_progressBarNewServerConfiguringMaximium = progressBarNewServerConfiguringMaximium;
        emit progressBarNewServerConfiguringMaximiumChanged();
    }
}

bool UiLogic::getProgressBarNewServerConfiguringTextVisible() const
{
    return m_progressBarNewServerConfiguringTextVisible;
}

void UiLogic::setProgressBarNewServerConfiguringTextVisible(bool progressBarNewServerConfiguringTextVisible)
{
    if (m_progressBarNewServerConfiguringTextVisible != progressBarNewServerConfiguringTextVisible) {
        m_progressBarNewServerConfiguringTextVisible = progressBarNewServerConfiguringTextVisible;
        emit progressBarNewServerConfiguringTextVisibleChanged();
    }
}

QString UiLogic::getProgressBarNewServerConfiguringText() const
{
    return m_progressBarNewServerConfiguringText;
}

void UiLogic::setProgressBarNewServerConfiguringText(const QString &progressBarNewServerConfiguringText)
{
    if (m_progressBarNewServerConfiguringText != progressBarNewServerConfiguringText) {
        m_progressBarNewServerConfiguringText = progressBarNewServerConfiguringText;
        emit progressBarNewServerConfiguringTextChanged();
    }
}

bool UiLogic::getPageServerProtocolsEnabled() const
{
    return m_pageServerProtocolsEnabled;
}

void UiLogic::setPageServerProtocolsEnabled(bool pageServerProtocolsEnabled)
{
    if (m_pageServerProtocolsEnabled != pageServerProtocolsEnabled) {
        m_pageServerProtocolsEnabled = pageServerProtocolsEnabled;
        emit pageServerProtocolsEnabledChanged();
    }
}

int UiLogic::getProgressBarProtocolsContainerReinstallValue() const
{
    return m_progressBarProtocolsContainerReinstallValue;
}

void UiLogic::setProgressBarProtocolsContainerReinstallValue(int progressBarProtocolsContainerReinstallValue)
{
    if (m_progressBarProtocolsContainerReinstallValue != progressBarProtocolsContainerReinstallValue) {
        m_progressBarProtocolsContainerReinstallValue = progressBarProtocolsContainerReinstallValue;
        emit progressBarProtocolsContainerReinstallValueChanged();
    }
}

int UiLogic::getProgressBarProtocolsContainerReinstallMaximium() const
{
    return m_progressBarProtocolsContainerReinstallMaximium;
}

void UiLogic::setProgressBarProtocolsContainerReinstallMaximium(int progressBarProtocolsContainerReinstallMaximium)
{
    if (m_progressBarProtocolsContainerReinstallMaximium != progressBarProtocolsContainerReinstallMaximium) {
        m_progressBarProtocolsContainerReinstallMaximium = progressBarProtocolsContainerReinstallMaximium;
        emit progressBarProtocolsContainerReinstallMaximiumChanged();
    }
}








UiLogic::~UiLogic()
{
    hide();
    m_vpnConnection->disconnectFromVpn();
    for (int i = 0; i < 50; i++) {
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep(100);
        if (m_vpnConnection->isDisconnected()) {
            break;
        }
    }

    delete m_vpnConnection;

    qDebug() << "Application closed";
}

void UiLogic::showOnStartup()
{
    if (! m_settings.isStartMinimized()) {
        show();
    } else {
#if defined Q_OS_MACX
        setDockIconVisible(false);
#endif
    }
}


//void UiLogic::keyPressEvent(QKeyEvent *event)
//{
//    switch (event->key()) {
//    case Qt::Key_L:
//        if (!Debug::openLogsFolder()) {
//            QMessageBox::warning(this, APPLICATION_NAME, tr("Cannot open logs folder!"));
//        }
//        break;
//#ifdef QT_DEBUG
//    case Qt::Key_Q:
//        qApp->quit();
//        break;
//        //    case Qt::Key_0:
//        //        *((char*)-1) = 'x';
//        //        break;
//    case Qt::Key_H:
//        selectedServerIndex = m_settings.defaultServerIndex();
//        selectedDockerContainer = m_settings.defaultContainer(selectedServerIndex);

//        updateSharingPage(selectedServerIndex, m_settings.serverCredentials(selectedServerIndex), selectedDockerContainer);
//        goToPage(Page::ShareConnection);
//        break;
//#endif
//    case Qt::Key_C:
//        qDebug().noquote() << "Def server" << m_settings.defaultServerIndex() << m_settings.defaultContainerName(m_settings.defaultServerIndex());
//        //qDebug().noquote() << QJsonDocument(m_settings.containerConfig(m_settings.defaultServerIndex(), m_settings.defaultContainer(m_settings.defaultServerIndex()))).toJson();
//        qDebug().noquote() << QJsonDocument(m_settings.defaultServer()).toJson();
//        break;
//    case Qt::Key_A:
//        goToPage(Page::Start);
//        break;
//    case Qt::Key_S:
//        selectedServerIndex = m_settings.defaultServerIndex();
//        goToPage(Page::ServerSettings);
//        break;
//    case Qt::Key_P:
//        selectedServerIndex = m_settings.defaultServerIndex();
//        selectedDockerContainer = m_settings.defaultContainer(selectedServerIndex);
//        goToPage(Page::ServerVpnProtocols);
//        break;
//    case Qt::Key_T:
//        SshConfigurator::openSshTerminal(m_settings.serverCredentials(m_settings.defaultServerIndex()));
//        break;
//    case Qt::Key_Escape:
//        if (currentPage() == Page::Vpn) break;
//        if (currentPage() == Page::ServerConfiguring) break;
//        if (currentPage() == Page::Start && pagesStack.size() < 2) break;
//        if (currentPage() == Page::Sites &&
//                ui->tableView_sites->selectionModel()->selection().indexes().size() > 0) {
//            ui->tableView_sites->clearSelection();
//            break;
//        }

//        if (! ui->stackedWidget_main->isAnimationRunning() && ui->stackedWidget_main->currentWidget()->isEnabled()) {
//            closePage();
//        }
//    default:
//        ;
//    }
//}

void UiLogic::onCloseWindow()
{
    if (m_settings.serversCount() == 0) qApp->quit();
    else {
        hide();
    }
}



//void UiLogic::showEvent(QShowEvent *event)
//{
//#if defined Q_OS_MACX
//    if (!event->spontaneous()) {
//        setDockIconVisible(true);
//    }
//    if (needToHideCustomTitlebar) {
//        ui->widget_tittlebar->hide();
//        resize(width(), 640);
//        ui->stackedWidget_main->move(0,0);
//    }
//#endif
//}

//void UiLogic::hideEvent(QHideEvent *event)
//{
//#if defined Q_OS_MACX
//    if (!event->spontaneous()) {
//        setDockIconVisible(false);
//    }
//#endif
//}


QMap<DockerContainer, QJsonObject> UiLogic::getInstallConfigsFromProtocolsPage() const
{
    QJsonObject cloakConfig {
        { config_key::container, amnezia::containerToString(DockerContainer::OpenVpnOverCloak) },
        { config_key::cloak, QJsonObject {
                { config_key::port, getLineEditNewServerCloakPortText() },
                { config_key::site, getLineEditNewServerCloakSiteText() }}
        }
    };
    QJsonObject ssConfig {
        { config_key::container, amnezia::containerToString(DockerContainer::OpenVpnOverShadowSocks) },
        { config_key::shadowsocks, QJsonObject {
                { config_key::port, getLineEditNewServerSsPortText() },
                { config_key::cipher, getComboBoxNewServerSsCipherText() }}
        }
    };
    QJsonObject openVpnConfig {
        { config_key::container, amnezia::containerToString(DockerContainer::OpenVpn) },
        { config_key::openvpn, QJsonObject {
                { config_key::port, getlineEditNewServerOpenvpnPortText() },
                { config_key::transport_proto, getComboBoxNewServerOpenvpnProtoText() }}
        }
    };

    QMap<DockerContainer, QJsonObject> containers;

    if (getCheckBoxNewServerCloakChecked()) {
        containers.insert(DockerContainer::OpenVpnOverCloak, cloakConfig);
    }

    if (getCheckBoxNewServerSsChecked()) {
        containers.insert(DockerContainer::OpenVpnOverShadowSocks, ssConfig);
    }

    if (getCheckBoxNewServerOpenvpnChecked()) {
        containers.insert(DockerContainer::OpenVpn, openVpnConfig);
    }

    return containers;
}


void UiLogic::installServer(const QMap<DockerContainer, QJsonObject> &containers)
{
    if (containers.isEmpty()) return;

    goToPage(Page::ServerConfiguring);
    QEventLoop loop;
    QTimer::singleShot(500, &loop, SLOT(quit()));
    loop.exec();
    qApp->processEvents();

    PageFunc page_new_server_configuring;
    page_new_server_configuring.setEnabledFunc = [this] (bool enabled) -> void {
        setPageNewServerConfiguringEnabled(enabled);
    };
    ButtonFunc no_button;
    LabelFunc label_new_server_configuring_wait_info;
    label_new_server_configuring_wait_info.setTextFunc = [this] (const QString& text) -> void {
        setLabelNewServerConfiguringWaitInfoText(text);
    };
    label_new_server_configuring_wait_info.setVisibleFunc = [this] (bool visible) ->void {
        setLabelNewServerConfiguringWaitInfoVisible(visible);
    };
    ProgressFunc progressBar_new_server_configuring;
    progressBar_new_server_configuring.setVisibleFunc = [this] (bool visible) ->void {
        setProgressBarNewServerConfiguringVisible(visible);
    };
    progressBar_new_server_configuring.setValueFunc = [this] (int value) ->void {
        setProgressBarNewServerConfiguringValue(value);
    };
    progressBar_new_server_configuring.getValueFunc = [this] (void) -> int {
        return getProgressBarNewServerConfiguringValue();
    };
    progressBar_new_server_configuring.getMaximiumFunc = [this] (void) -> int {
        return getProgressBarNewServerConfiguringMaximium();
    };
    progressBar_new_server_configuring.setTextVisibleFunc = [this] (bool visible) ->void {
        setProgressBarNewServerConfiguringTextVisible(visible);
    };
    progressBar_new_server_configuring.setTextFunc = [this] (const QString& text) ->void {
        setProgressBarNewServerConfiguringText(text);
    };
    bool ok = installContainers(installCredentials, containers,
                                page_new_server_configuring,
                                progressBar_new_server_configuring,
                                no_button,
                                label_new_server_configuring_wait_info);

    if (ok) {
        QJsonObject server;
        server.insert(config_key::hostName, installCredentials.hostName);
        server.insert(config_key::userName, installCredentials.userName);
        server.insert(config_key::password, installCredentials.password);
        server.insert(config_key::port, installCredentials.port);
        server.insert(config_key::description, m_settings.nextAvailableServerName());

        QJsonArray containerConfigs;
        for (const QJsonObject &cfg : containers) {
            containerConfigs.append(cfg);
        }
        server.insert(config_key::containers, containerConfigs);
        server.insert(config_key::defaultContainer, containerToString(containers.firstKey()));

        m_settings.addServer(server);
        m_settings.setDefaultServer(m_settings.serversCount() - 1);

        setStartPage(Page::Vpn);
        qApp->processEvents();
    }
    else {
        closePage();
    }
}

bool UiLogic::installContainers(ServerCredentials credentials,
                                const QMap<DockerContainer, QJsonObject> &containers,
                                const PageFunc &page,
                                const ProgressFunc &progress,
                                const ButtonFunc &button,
                                const LabelFunc &info)
{
    if (!progress.setValueFunc) return false;

    if (page.setEnabledFunc) {
        page.setEnabledFunc(false);
    }
    if (button.setVisibleFunc) {
        button.setVisibleFunc(false);
    }

    if (info.setVisibleFunc) {
        info.setVisibleFunc(true);
    }
    if (info.setTextFunc) {
        info.setTextFunc(tr("Please wait, configuring process may take up to 5 minutes"));
    }

    int cnt = 0;
    for (QMap<DockerContainer, QJsonObject>::const_iterator i = containers.constBegin(); i != containers.constEnd(); i++, cnt++) {
        QTimer timer;
        connect(&timer, &QTimer::timeout, [progress](){
            progress.setValueFunc(progress.getValueFunc() + 1);
        });

        progress.setValueFunc(0);
        timer.start(1000);

        progress.setTextVisibleFunc(true);
        progress.setTextFunc(QString("Installing %1 %2 %3").arg(cnt+1).arg(tr("of")).arg(containers.size()));

        ErrorCode e = ServerController::setupContainer(credentials, i.key(), i.value());
        qDebug() << "Setup server finished with code" << e;
        ServerController::disconnectFromHost(credentials);

        if (e) {
            if (page.setEnabledFunc) {
                page.setEnabledFunc(true);
            }
            if (button.setVisibleFunc) {
                button.setVisibleFunc(true);
            }
            if (info.setVisibleFunc) {
                info.setVisibleFunc(false);
            }

            QMessageBox::warning(nullptr, APPLICATION_NAME,
                                 tr("Error occurred while configuring server.") + "\n" +
                                 errorString(e));

            return false;
        }

        // just ui progressbar tweak
        timer.stop();

        int remaining_val = progress.getMaximiumFunc() - progress.getValueFunc();

        if (remaining_val > 0) {
            QTimer timer1;
            QEventLoop loop1;

            connect(&timer1, &QTimer::timeout, [&](){
                progress.setValueFunc(progress.getValueFunc() + 1);
                if (progress.getValueFunc() >= progress.getMaximiumFunc()) {
                    loop1.quit();
                }
            });

            timer1.start(5);
            loop1.exec();
        }
    }


    if (button.setVisibleFunc) {
        button.setVisibleFunc(true);
    }
    if (page.setEnabledFunc) {
        page.setEnabledFunc(true);
    }
    if (info.setTextFunc) {
        info.setTextFunc(tr("Amnezia server installed"));
    }

    return true;
}

ErrorCode UiLogic::doInstallAction(const std::function<ErrorCode()> &action,
                                   const PageFunc &page,
                                   const ProgressFunc &progress,
                                   const ButtonFunc &button,
                                   const LabelFunc &info)
{
    progress.setVisibleFunc(true);
    if (page.setEnabledFunc) {
        page.setEnabledFunc(false);
    }
    if (button.setVisibleFunc) {
        button.setVisibleFunc(false);
    }
    if (info.setVisibleFunc) {
        info.setVisibleFunc(true);
    }
    if (info.setTextFunc) {
        info.setTextFunc(tr("Please wait, configuring process may take up to 5 minutes"));
    }

    QTimer timer;
    connect(&timer, &QTimer::timeout, [progress](){
        progress.setValueFunc(progress.getValueFunc() + 1);
    });

    progress.setValueFunc(0);
    timer.start(1000);

    ErrorCode e = action();
    qDebug() << "doInstallAction finished with code" << e;

    if (e) {
        if (page.setEnabledFunc) {
            page.setEnabledFunc(true);
        }
        if (button.setVisibleFunc) {
            button.setVisibleFunc(true);
        }
        if (info.setVisibleFunc) {
            info.setVisibleFunc(false);
        }
        QMessageBox::warning(nullptr, APPLICATION_NAME,
                             tr("Error occurred while configuring server.") + "\n" +
                             errorString(e));

        progress.setVisibleFunc(false);
        return e;
    }

    // just ui progressbar tweak
    timer.stop();

    int remaining_val = progress.getMaximiumFunc() - progress.getValueFunc();

    if (remaining_val > 0) {
        QTimer timer1;
        QEventLoop loop1;

        connect(&timer1, &QTimer::timeout, [&](){
            progress.setValueFunc(progress.getValueFunc() + 1);
            if (progress.getValueFunc() >= progress.getMaximiumFunc()) {
                loop1.quit();
            }
        });

        timer1.start(5);
        loop1.exec();
    }


    progress.setVisibleFunc(false);
    if (button.setVisibleFunc) {
        button.setVisibleFunc(true);
    }
    if (page.setEnabledFunc) {
        page.setEnabledFunc(true);
    }
    if (info.setTextFunc) {
        info.setTextFunc(tr("Operation finished"));
    }
    return ErrorCode::NoError;
}


void UiLogic::onBytesChanged(quint64 receivedData, quint64 sentData)
{
    setLabelSpeedReceivedText(VpnConnection::bytesPerSecToText(receivedData));
    setLabelSpeedSentText(VpnConnection::bytesPerSecToText(sentData));
}

void UiLogic::onConnectionStateChanged(VpnProtocol::ConnectionState state)
{
    qDebug() << "UiLogic::onConnectionStateChanged" << VpnProtocol::textConnectionState(state);

    bool pushButtonConnectEnabled = false;
    bool radioButtonsModeEnabled = false;
    setLabelStateText(VpnProtocol::textConnectionState(state));

    setTrayState(state);

    switch (state) {
    case VpnProtocol::Disconnected:
        onBytesChanged(0,0);
        setPushButtonConnectChecked(false);
        pushButtonConnectEnabled = true;
        radioButtonsModeEnabled = true;
        break;
    case VpnProtocol::Preparing:
        pushButtonConnectEnabled = false;
        radioButtonsModeEnabled = false;
        break;
    case VpnProtocol::Connecting:
        pushButtonConnectEnabled = false;
        radioButtonsModeEnabled = false;
        break;
    case VpnProtocol::Connected:
        pushButtonConnectEnabled = true;
        radioButtonsModeEnabled = false;
        break;
    case VpnProtocol::Disconnecting:
        pushButtonConnectEnabled = false;
        radioButtonsModeEnabled = false;
        break;
    case VpnProtocol::Reconnecting:
        pushButtonConnectEnabled = true;
        radioButtonsModeEnabled = false;
        break;
    case VpnProtocol::Error:
        setPushButtonConnectEnabled(false);
        pushButtonConnectEnabled = true;
        radioButtonsModeEnabled = true;
        break;
    case VpnProtocol::Unknown:
        pushButtonConnectEnabled = true;
        radioButtonsModeEnabled = true;
    }

    setPushButtonConnectEnabled(pushButtonConnectEnabled);
    setWidgetVpnModeEnabled(radioButtonsModeEnabled);
}

void UiLogic::onVpnProtocolError(ErrorCode errorCode)
{
    setLabelErrorText(errorString(errorCode));
}

void UiLogic::onPushButtonConnectClicked(bool checked)
{
    if (checked) {
        onConnect();
    } else {
        onDisconnect();
    }
}

void UiLogic::setupTray()
{
    setTrayState(VpnProtocol::Disconnected);
}

void UiLogic::setTrayIcon(const QString &iconPath)
{
    setTrayIconUrl(iconPath);
}

PageEnumNS::Page UiLogic::currentPage()
{
    return static_cast<PageEnumNS::Page>(getCurrentPageValue());
}

void UiLogic::setupNewServerConnections()
{
    connect(this, &UiLogic::pushButtonNewServerConnectConfigureClicked, this, [this](){
        installServer(getInstallConfigsFromProtocolsPage());
    });
}

void UiLogic::setupProtocolsPageConnections()
{
    QJsonObject openvpnConfig;

    // all containers
    QList<DockerContainer> containers {
        DockerContainer::OpenVpn,
                DockerContainer::OpenVpnOverShadowSocks,
                DockerContainer::OpenVpnOverCloak,
                DockerContainer::WireGuard
    };
    using ButtonClickedFunc = void (UiLogic::*)(bool);
    using ButtonSetEnabledFunc = std::function<void(bool)>;

    // default buttons
    QList<ButtonClickedFunc> defaultButtonClickedSig {
        &UiLogic::pushButtonProtoOpenvpnContDefaultClicked,
                &UiLogic::pushButtonProtoSsOpenvpnContDefaultClicked,
                &UiLogic::pushButtonProtoCloakOpenvpnContDefaultClicked,
                &UiLogic::pushButtonProtoWireguardContDefaultClicked
    };

    for (int i = 0; i < containers.size(); ++i) {
        connect(this, defaultButtonClickedSig.at(i), [this, containers, i](bool){
            qDebug() << "clmm" << i;
            m_settings.setDefaultContainer(selectedServerIndex, containers.at(i));
            updateProtocolsPage();
        });
    }

    // install buttons
    QList<ButtonClickedFunc> installButtonsClickedSig {
        &UiLogic::pushButtonProtoOpenvpnContInstallClicked,
                &UiLogic::pushButtonProtoSsOpenvpnContInstallClicked,
                &UiLogic::pushButtonProtoCloakOpenvpnContInstallClicked,
                &UiLogic::pushButtonProtoWireguardContInstallClicked,
    };
    QList<ButtonSetEnabledFunc> installButtonsSetEnabledFunc {
        [this] (bool enabled) -> void {
            setPushButtonProtoOpenvpnContInstallEnabled(enabled);
        },
        [this] (bool enabled) -> void {
            setPushButtonProtoSsOpenvpnContInstallEnabled(enabled);
        },
        [this] (bool enabled) -> void {
            setPushButtonProtoCloakOpenvpnContInstallEnabled(enabled);
        },
        [this] (bool enabled) -> void {
            setPushButtonProtoWireguardContInstallEnabled(enabled);
        },
    };

    for (int i = 0; i < containers.size(); ++i) {
        ButtonClickedFunc buttonClickedFunc = installButtonsClickedSig.at(i);
        ButtonSetEnabledFunc buttonSetEnabledFunc = installButtonsSetEnabledFunc.at(i);
        DockerContainer container = containers.at(i);

        connect(this, buttonClickedFunc, [this, container, buttonSetEnabledFunc](bool checked){
            if (checked) {
                PageFunc page_server_protocols;
                page_server_protocols.setEnabledFunc = [this] (bool enabled) -> void {
                    setPageServerProtocolsEnabled(enabled);
                };
                ButtonFunc no_button;
                LabelFunc no_label;
                ProgressFunc progressBar_protocols_container_reinstall;
                progressBar_protocols_container_reinstall.setVisibleFunc = [this] (bool visible) ->void {
                    setProgressBarProtocolsContainerReinstallVisible(visible);
                };
                progressBar_protocols_container_reinstall.setValueFunc = [this] (int value) ->void {
                    setProgressBarProtocolsContainerReinstallValue(value);
                };
                progressBar_protocols_container_reinstall.getValueFunc = [this] (void) -> int {
                    return getProgressBarProtocolsContainerReinstallValue();
                };
                progressBar_protocols_container_reinstall.getMaximiumFunc = [this] (void) -> int {
                    return getProgressBarProtocolsContainerReinstallMaximium();
                };

                ErrorCode e = doInstallAction([this, container](){
                    return ServerController::setupContainer(m_settings.serverCredentials(selectedServerIndex), container);
                },
                page_server_protocols, progressBar_protocols_container_reinstall,
                no_button, no_label);

                if (!e) {
                    m_settings.setContainerConfig(selectedServerIndex, container, QJsonObject());
                    m_settings.setDefaultContainer(selectedServerIndex, container);
                }
            }
            else {
                buttonSetEnabledFunc(false);
                ErrorCode e = ServerController::removeContainer(m_settings.serverCredentials(selectedServerIndex), container);
                m_settings.removeContainerConfig(selectedServerIndex, container);
                buttonSetEnabledFunc(true);

                if (m_settings.defaultContainer(selectedServerIndex) == container) {
                    const auto &c = m_settings.containers(selectedServerIndex);
                    if (c.isEmpty()) m_settings.setDefaultContainer(selectedServerIndex, DockerContainer::None);
                    else m_settings.setDefaultContainer(selectedServerIndex, c.keys().first());
                }
            }

            updateProtocolsPage();
        });
    }

    // share buttons
    QList<ButtonClickedFunc> shareButtonsClickedSig {
        &UiLogic::pushButtonProtoOpenvpnContShareClicked,
                &UiLogic::pushButtonProtoSsOpenvpnContShareClicked,
                &UiLogic::pushButtonProtoCloakOpenvpnContShareClicked,
                &UiLogic::pushButtonProtoWireguardContShareClicked,
    };

    for (int i = 0; i < containers.size(); ++i) {
        ButtonClickedFunc buttonClickedFunc = shareButtonsClickedSig.at(i);
        DockerContainer container = containers.at(i);

        connect(this, buttonClickedFunc, [this, container](bool){
            shareConnectionLogic()->updateSharingPage(selectedServerIndex, m_settings.serverCredentials(selectedServerIndex), container);
            goToPage(Page::ShareConnection);
        });
    }
}

void UiLogic::setTrayState(VpnProtocol::ConnectionState state)
{
    QString resourcesPath = "qrc:/images/tray/%1";

    setTrayActionDisconnectEnabled(state == VpnProtocol::Connected);
    setTrayActionConnectEnabled(state == VpnProtocol::Disconnected);

    switch (state) {
    case VpnProtocol::Disconnected:
        setTrayIcon(QString(resourcesPath).arg(DisconnectedTrayIconName));
        break;
    case VpnProtocol::Preparing:
        setTrayIcon(QString(resourcesPath).arg(DisconnectedTrayIconName));
        break;
    case VpnProtocol::Connecting:
        setTrayIcon(QString(resourcesPath).arg(DisconnectedTrayIconName));
        break;
    case VpnProtocol::Connected:
        setTrayIcon(QString(resourcesPath).arg(ConnectedTrayIconName));
        break;
    case VpnProtocol::Disconnecting:
        setTrayIcon(QString(resourcesPath).arg(DisconnectedTrayIconName));
        break;
    case VpnProtocol::Reconnecting:
        setTrayIcon(QString(resourcesPath).arg(DisconnectedTrayIconName));
        break;
    case VpnProtocol::Error:
        setTrayIcon(QString(resourcesPath).arg(ErrorTrayIconName));
        break;
    case VpnProtocol::Unknown:
    default:
        setTrayIcon(QString(resourcesPath).arg(DisconnectedTrayIconName));
    }

    //#ifdef Q_OS_MAC
    //    // Get theme from current user (note, this app can be launched as root application and in this case this theme can be different from theme of real current user )
    //    bool darkTaskBar = MacOSFunctions::instance().isMenuBarUseDarkTheme();
    //    darkTaskBar = forceUseBrightIcons ? true : darkTaskBar;
    //    resourcesPath = ":/images_mac/tray_icon/%1";
    //    useIconName = useIconName.replace(".png", darkTaskBar ? "@2x.png" : " dark@2x.png");
    //#endif

}

void UiLogic::onConnect()
{
    int serverIndex = m_settings.defaultServerIndex();
    ServerCredentials credentials = m_settings.serverCredentials(serverIndex);
    DockerContainer container = m_settings.defaultContainer(serverIndex);

    if (m_settings.containers(serverIndex).isEmpty()) {
        setLabelErrorText(tr("VPN Protocols is not installed.\n Please install VPN container at first"));
        setPushButtonConnectChecked(false);
        return;
    }

    if (container == DockerContainer::None) {
        setLabelErrorText(tr("VPN Protocol not choosen"));
        setPushButtonConnectChecked(false);
        return;
    }


    const QJsonObject &containerConfig = m_settings.containerConfig(serverIndex, container);
    onConnectWorker(serverIndex, credentials, container, containerConfig);
}

void UiLogic::onConnectWorker(int serverIndex, const ServerCredentials &credentials, DockerContainer container, const QJsonObject &containerConfig)
{
    setLabelErrorText("");
    setPushButtonConnectChecked(true);
    qApp->processEvents();

    ErrorCode errorCode = m_vpnConnection->connectToVpn(
                serverIndex, credentials, container, containerConfig
                );

    if (errorCode) {
        //ui->pushButton_connect->setChecked(false);
        setDialogConnectErrorText(errorString(errorCode));
        emit showConnectErrorDialog();
        return;
    }

    setPushButtonConnectEnabled(false);
}

void UiLogic::onDisconnect()
{
    setPushButtonConnectChecked(false);
    m_vpnConnection->disconnectFromVpn();
}








void UiLogic::onPushButtonProtoOpenvpnContOpenvpnConfigClicked()
{
    selectedDockerContainer = DockerContainer::OpenVpn;
    m_openVpnLogic->updateOpenVpnPage(m_settings.protocolConfig(selectedServerIndex, selectedDockerContainer, Protocol::OpenVpn),
                      selectedDockerContainer, m_settings.haveAuthData(selectedServerIndex));
    goToPage(Page::OpenVpnSettings);
}

void UiLogic::onPushButtonProtoSsOpenvpnContOpenvpnConfigClicked()
{
    selectedDockerContainer = DockerContainer::OpenVpnOverShadowSocks;
    m_openVpnLogic->updateOpenVpnPage(m_settings.protocolConfig(selectedServerIndex, selectedDockerContainer, Protocol::OpenVpn),
                      selectedDockerContainer, m_settings.haveAuthData(selectedServerIndex));
    goToPage(Page::OpenVpnSettings);
}

void UiLogic::onPushButtonProtoSsOpenvpnContSsConfigClicked()
{
    selectedDockerContainer = DockerContainer::OpenVpnOverShadowSocks;
    shadowSocksLogic()->updateShadowSocksPage(m_settings.protocolConfig(selectedServerIndex, selectedDockerContainer, Protocol::ShadowSocks),
                          selectedDockerContainer, m_settings.haveAuthData(selectedServerIndex));
    goToPage(Page::ShadowSocksSettings);
}






void UiLogic::updateVpnPage()
{
    Settings::RouteMode mode = m_settings.routeMode();
    setRadioButtonVpnModeAllSitesChecked(mode == Settings::VpnAllSites);
    setRadioButtonVpnModeForwardSitesChecked(mode == Settings::VpnOnlyForwardSites);
    setRadioButtonVpnModeExceptSitesChecked(mode == Settings::VpnAllExceptSites);
    setPushButtonVpnAddSiteEnabled(mode != Settings::VpnAllSites);
}









void UiLogic::onRadioButtonVpnModeAllSitesToggled(bool checked)
{
    if (checked) {
        m_settings.setRouteMode(Settings::VpnAllSites);
    }
}

void UiLogic::onRadioButtonVpnModeForwardSitesToggled(bool checked)
{
    if (checked) {
        m_settings.setRouteMode(Settings::VpnOnlyForwardSites);
    }
}

void UiLogic::onRadioButtonVpnModeExceptSitesToggled(bool checked)
{
    if (checked) {
        m_settings.setRouteMode(Settings::VpnAllExceptSites);
    }
}




void UiLogic::updateProtocolsPage()
{
    setProgressBarProtocolsContainerReinstallVisible(false);

    auto containers = m_settings.containers(selectedServerIndex);
    DockerContainer defaultContainer = m_settings.defaultContainer(selectedServerIndex);
    bool haveAuthData = m_settings.haveAuthData(selectedServerIndex);

    // all containers
    QList<DockerContainer> allContainers {
        DockerContainer::OpenVpn,
                DockerContainer::OpenVpnOverShadowSocks,
                DockerContainer::OpenVpnOverCloak,
                DockerContainer::WireGuard
    };

    using SetVisibleFunc = std::function<void(bool)>;
    using SetCheckedFunc = std::function<void(bool)>;
    using SetEnabledFunc = std::function<void(bool)>;
    QList<SetCheckedFunc> installButtonsCheckedFunc {
        [this](bool checked) ->void {setPushButtonProtoOpenvpnContInstallChecked(checked);},
        [this](bool checked) ->void {setPushButtonProtoSsOpenvpnContInstallChecked(checked);},
        [this](bool checked) ->void {setPushButtonProtoCloakOpenvpnContInstallChecked(checked);},
        [this](bool checked) ->void {setPushButtonProtoWireguardContInstallChecked(checked);},
    };
    QList<SetEnabledFunc> installButtonsEnabledFunc {
        [this](bool enabled) ->void {setPushButtonProtoOpenvpnContInstallEnabled(enabled);},
        [this](bool enabled) ->void {setPushButtonProtoSsOpenvpnContInstallEnabled(enabled);},
        [this](bool enabled) ->void {setPushButtonProtoCloakOpenvpnContInstallEnabled(enabled);},
        [this](bool enabled) ->void {setPushButtonProtoWireguardContInstallEnabled(enabled);},
    };

    QList<SetCheckedFunc> defaultButtonsCheckedFunc {
        [this](bool checked) ->void {setPushButtonProtoOpenvpnContDefaultChecked(checked);},
        [this](bool checked) ->void {setPushButtonProtoSsOpenvpnContDefaultChecked(checked);},
        [this](bool checked) ->void {setPushButtonProtoCloakOpenvpnContDefaultChecked(checked);},
        [this](bool checked) ->void {setPushButtonProtoWireguardContDefaultChecked(checked);},
    };
    QList<SetVisibleFunc> defaultButtonsVisibleFunc {
        [this](bool visible) ->void {setPushButtonProtoOpenvpnContDefaultVisible(visible);},
        [this](bool visible) ->void {setPushButtonProtoSsOpenvpnContDefaultVisible(visible);},
        [this](bool visible) ->void {setPushButtonProtoCloakOpenvpnContDefaultVisible(visible);},
        [this](bool visible) ->void {setPushButtonProtoWireguardContDefaultVisible(visible);},
    };

    QList<SetVisibleFunc> shareButtonsVisibleFunc {
        [this](bool visible) ->void {setPushButtonProtoOpenvpnContShareVisible(visible);},
        [this](bool visible) ->void {setPushButtonProtoSsOpenvpnContShareVisible(visible);},
        [this](bool visible) ->void {setPushButtonProtoCloakOpenvpnContShareVisible(visible);},
        [this](bool visible) ->void {setPushButtonProtoWireguardContShareVisible(visible);},
    };

    QList<SetVisibleFunc> framesVisibleFunc {
        [this](bool visible) ->void {setFrameOpenvpnSettingsVisible(visible);},
        [this](bool visible) ->void {setFrameOpenvpnSsSettingsVisible(visible);},
        [this](bool visible) ->void {setFrameOpenvpnSsCloakSettingsVisible(visible);},
        [this](bool visible) ->void {setFrameWireguardSettingsVisible(visible);},
    };

    for (int i = 0; i < allContainers.size(); ++i) {
        defaultButtonsCheckedFunc.at(i)(defaultContainer == allContainers.at(i));
        defaultButtonsVisibleFunc.at(i)(haveAuthData && containers.contains(allContainers.at(i)));
        shareButtonsVisibleFunc.at(i)(haveAuthData && containers.contains(allContainers.at(i)));
        installButtonsCheckedFunc.at(i)(containers.contains(allContainers.at(i)));
        installButtonsEnabledFunc.at(i)(haveAuthData);
        framesVisibleFunc.at(i)(containers.contains(allContainers.at(i)));
    }
}


void UiLogic::onPushButtonProtoCloakOpenvpnContOpenvpnConfigClicked()
{
    selectedDockerContainer = DockerContainer::OpenVpnOverCloak;
    m_openVpnLogic->updateOpenVpnPage(m_settings.protocolConfig(selectedServerIndex, selectedDockerContainer, Protocol::OpenVpn),
                      selectedDockerContainer, m_settings.haveAuthData(selectedServerIndex));
    goToPage(Page::OpenVpnSettings);
}

void UiLogic::onPushButtonProtoCloakOpenvpnContSsConfigClicked()
{
    selectedDockerContainer = DockerContainer::OpenVpnOverCloak;
    shadowSocksLogic()->updateShadowSocksPage(m_settings.protocolConfig(selectedServerIndex, selectedDockerContainer, Protocol::ShadowSocks),
                          selectedDockerContainer, m_settings.haveAuthData(selectedServerIndex));
    goToPage(Page::ShadowSocksSettings);
}

void UiLogic::onPushButtonProtoCloakOpenvpnContCloakConfigClicked()
{
    selectedDockerContainer = DockerContainer::OpenVpnOverCloak;
    cloakLogic()->updateCloakPage(m_settings.protocolConfig(selectedServerIndex, selectedDockerContainer, Protocol::Cloak),
                    selectedDockerContainer, m_settings.haveAuthData(selectedServerIndex));
    goToPage(Page::CloakSettings);
}
