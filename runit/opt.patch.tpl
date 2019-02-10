diff --git a/runit.c b/runit.c
index 48620b3..b5d3c5c 100644
--- a/runit.c
+++ b/runit.c
@@ -23,9 +23,9 @@
 #define FATAL "- runit: fatal: "
 
 const char * const stage[3] ={
-  "/etc/runit/1",
-  "/etc/runit/2",
-  "/etc/runit/3" };
+  "%INSTALL_DIR%/etc/runit/1",
+  "%INSTALL_DIR%/etc/runit/2",
+  "%INSTALL_DIR%/etc/runit/3" };
 
 int selfpipe[2];
 int sigc =0;
diff --git a/runit.h b/runit.h
index ba98386..b7034e7 100644
--- a/runit.h
+++ b/runit.h
@@ -1,4 +1,4 @@
-#define RUNIT "/sbin/runit"
-#define STOPIT "/etc/runit/stopit"
-#define REBOOT "/etc/runit/reboot"
-#define CTRLALTDEL "/etc/runit/ctrlaltdel"
+#define RUNIT "%INSTALL_DIR%/bin/runit"
+#define STOPIT "%INSTALL_DIR%/etc/runit/stopit"
+#define REBOOT "%INSTALL_DIR%/etc/runit/reboot"
+#define CTRLALTDEL "%INSTALL_DIR%/etc/runit/ctrlaltdel"
diff --git a/runsvdir.c b/runsvdir.c
index 07c1d8e..a4bea53 100644
--- a/runsvdir.c
+++ b/runsvdir.c
@@ -65,7 +65,7 @@ void runsv(int no, char *name) {
     /* child */
     const char *prog[3];
 
-    prog[0] ="runsv";
+    prog[0] ="%INSTALL_DIR%/bin/runsv";
     prog[1] =name;
     prog[2] =0;
     sig_uncatch(sig_hangup);
diff --git a/sv.c b/sv.c
index 0125795..dd6d49e 100644
--- a/sv.c
+++ b/sv.c
@@ -32,7 +32,7 @@
 char *progname;
 char *action;
 char *acts;
-char *varservice ="/service/";
+char *varservice ="%INSTALL_DIR%/service/";
 char **service;
 char **servicex;
 unsigned int services;
