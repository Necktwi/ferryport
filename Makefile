#
#  There exist several targets which are by default empty and which can be 
#  used for execution of your targets. These targets are usually executed 
#  before and after some main targets. They are: 
#
#     .build-pre:              called before 'build' target
#     .build-post:             called after 'build' target
#     .clean-pre:              called before 'clean' target
#     .clean-post:             called after 'clean' target
#     .clobber-pre:            called before 'clobber' target
#     .clobber-post:           called after 'clobber' target
#     .all-pre:                called before 'all' target
#     .all-post:               called after 'all' target
#     .help-pre:               called before 'help' target
#     .help-post:              called after 'help' target
#
#  Targets beginning with '.' are not intended to be called on their own.
#
#  Main targets can be executed directly, and they are:
#  
#     build                    build a specific configuration
#     clean                    remove built files from a configuration
#     clobber                  remove all built files
#     all                      build all configurations
#     help                     print help mesage
#  
#  Targets .build-impl, .clean-impl, .clobber-impl, .all-impl, and
#  .help-impl are implemented in nbproject/makefile-impl.mk.
#
#  Available make variables:
#
#     CND_BASEDIR                base directory for relative paths
#     CND_DISTDIR                default top distribution directory (build artifacts)
#     CND_BUILDDIR               default top build directory (object files, ...)
#     CONF                       name of current configuration
#     CND_PLATFORM_${CONF}       platform name (current configuration)
#     CND_ARTIFACT_DIR_${CONF}   directory of build artifact (current configuration)
#     CND_ARTIFACT_NAME_${CONF}  name of build artifact (current configuration)
#     CND_ARTIFACT_PATH_${CONF}  path to build artifact (current configuration)
#     CND_PACKAGE_DIR_${CONF}    directory of package (current configuration)
#     CND_PACKAGE_NAME_${CONF}   name of package (current configuration)
#     CND_PACKAGE_PATH_${CONF}   path to package (current configuration)
#
# NOCDDL


# Environment 
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
APPNAME=ferryport
DEBUG=true


# build
build: .build-post

.build-pre:
# Add your pre 'build' code here...

.build-post: .build-impl
# Add your post 'build' code here...
	echo ${CND_ARTIFACT_PATH_${CONF}}
	
# clean0
clean: .clean-post

.clean-pre:
# Add your pre 'clean' code here...
#	sh ffmpeg_clean.sh
	rm -Rf ffmpeg_build
	
.clean-post: .clean-impl
# Add your post 'clean' code here...
	
# clobber
clobber: .clobber-post

.clobber-pre:
# Add your pre 'clobber' code here...

.clobber-post: .clobber-impl
# Add your post 'clobber' code here...


# all
all: .all-post

.all-pre:
# Add your pre 'all' code here...

.all-post: .all-impl
# Add your post 'all' code here...

# build tests
build-tests: .build-tests-post

.build-tests-pre:
# Add your pre 'build-tests' code here...

.build-tests-post: .build-tests-impl
# Add your post 'build-tests' code here...


# run tests
test: .test-post

.test-pre: build-tests
# Add your pre 'test' code here...

.test-post: .test-impl
# Add your post 'test' code here...


# help
help: .help-post

.help-pre:
# Add your pre 'help' code here...

.help-post: .help-impl
# Add your post 'help' code here...

install:
	install -m644 -D config.xml $(DESTDIR)/etc/${APPNAME}.conf.xml
	install -m644 -D wvdial.conf $(DESTDIR)/etc/wvdial.conf
	install -m644 -D devices.rules $(DESTDIR)/lib/udev/rules.d/50-${APPNAME}.rules
	install -m644 -D init.conf $(DESTDIR)/etc/init/${APPNAME}.conf
	install -m644 -D init.override $(DESTDIR)/etc/init/${APPNAME}.override
	install -m644 -D certs/ferryfair.cert $(DESTDIR)/etc/ssl/certs/ferryfair.cert
	install -m644 -D certs/ferryport.ferryfair.cert $(DESTDIR)/etc/ssl/certs/${CND_ARTIFACT_NAME_${CONF}}.ferryfair.cert
	install -m644 -D certs/ferryport.ferryfair.key $(DESTDIR)/etc/ssl/certs/${CND_ARTIFACT_NAME_${CONF}}.ferryfair.key
	install -m644 -D ttyO1_armhf.com-00A0.dtbo $(DESTDIR)/lib/firmware/ttyO1_armhf.com-00A0.dtbo
	install -m755 -D ${CND_ARTIFACT_PATH_${CONF}} $(DESTDIR)/usr/bin/${APPNAME}
	install -m644 -D man $(DESTDIR)/usr/share/man/man1/$(APPNAME).1
	
install-exec-hook:
	${CND_ARTIFACT_PATH_${CONF}} -i
	
mytest:
	./${CND_ARTIFACT_PATH_${CONF}} -t
	
debpackage:
	./ffmpeg_install.sh
	
# include project implementation makefile
include nbproject/Makefile-impl.mk

# include project make variables
include nbproject/Makefile-variables.mk
