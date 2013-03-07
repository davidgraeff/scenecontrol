exports.runtimeconfig = {
	"ROOM_BASEPATH": "@CONFIG_SERVER_BASEPATH@",
	"ROOM_LIBPATH": "@CONFIG_SERVER_LIBPATH@",
	"ROOM_CERTPATH": "@CONFIG_SERVER_CERTPATH@",
	"ROOM_CERTPATH_CLIENTS": "@CONFIG_CLIENT_CERTPATH@",
	"ROOM_DATABASEIMPORTPATH": "@CONFIG_SERVER_DATABASEIMPORTPATH@",
	"ROOM_LISTENPORT @CONFIG_SERVER_LISTENPORT@
	"ROOM_WEBSOCKETPROXY_LISTENPORT @CONFIG_WEBSOCKETPROXY_LISTENPORT@
	"ROOM_LOGFILE": "@CONFIG_SERVER_LOGFILE@"
};
exports.aboutconfig = {
	"ABOUT_SERVICENAME": "@CPACK_PACKAGE_NAME@",
	"ABOUT_AUTHOR": "@CPACK_PACKAGE_VENDOR@",
	"ABOUT_AUTHOR_EMAIL": "@CPACK_PACKAGE_CONTACT@",
	"ABOUT_ORGANIZATIONID": "@CPACK_PACKAGE_NAME@",
	"ABOUT_VERSION": "@CPACK_PACKAGE_VERSION_MAJOR@.@CPACK_PACKAGE_VERSION_MINOR@.@CPACK_PACKAGE_VERSION_PATCH@",
	"ABOUT_LASTCOMMITDATE": "@LASTCOMMITDATE@",
	"ABOUT_COMMITHASH": "@COMMITHASH@"
};
