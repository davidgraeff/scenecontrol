exports.runtimeconfig = {
	"controlport": 3102,
	"websocketport": 3103,
	"sslcertificatekey": "1234",
	"databasename": "testdb"
};

exports.systempaths = {
	"path_plugins": "/usr/local/lib/scenecontrol_suite",
	"path_cert_server": "/usr/local/share/scenecontrol_suite/cert",
	"path_certs_clients": "/usr/local/share/scenecontrol_suite/cert/services",
	"path_database_files": "/usr/local/share/scenecontrol_suite/dbimport"
};

exports.userpaths = {
	"path_cert_server": "./certificates",
	"path_certs_clients": "../services/_shared/certificates"
};

exports.aboutconfig = {
	"ABOUT_SERVICENAME": "SceneServer",
	"ABOUT_AUTHOR": "David Gräff",
	"ABOUT_AUTHOR_EMAIL": "david.graeff.at.web.de",
	"ABOUT_ORGANIZATIONID": "",
	"ABOUT_VERSION": "3.0.0",
	"ABOUT_LASTCOMMITDATE": "",
	"ABOUT_COMMITHASH": ""
};
