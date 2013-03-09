exports.runtimeconfig = {
	"path_system_plugins": "/usr/local/lib/scenecontrol_suite",
	"path_system_server_cert": "/usr/local/share/scenecontrol_suite/cert",
	"path_user_server_cert": "./certificates",
	"path_certs_clients": "/usr/local/share/scenecontrol_suite/cert/clients",
	"path_system_database_files": "/usr/local/share/scenecontrol_suite/dbimport",
	"controlport": 3102,
	"websocketport": 3103,
	"databasename": "testdb"
};
exports.aboutconfig = {
	"ABOUT_SERVICENAME": "SceneServer",
	"ABOUT_AUTHOR": "@CPACK_PACKAGE_VENDOR@",
	"ABOUT_AUTHOR_EMAIL": "@CPACK_PACKAGE_CONTACT@",
	"ABOUT_ORGANIZATIONID": "@CPACK_PACKAGE_NAME@",
	"ABOUT_VERSION": "3.0.0",
	"ABOUT_LASTCOMMITDATE": "@LASTCOMMITDATE@",
	"ABOUT_COMMITHASH": "@COMMITHASH@"
};
