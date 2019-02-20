diff --git a/runit.c b/runit.c
index 48620b3..c2067d8 100644
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
@@ -62,12 +62,14 @@ int main (int argc, const char * const *argv, char * const *envp) {
   sig_block(sig_child);
   sig_catch(sig_child, sig_child_handler);
   sig_block(sig_cont);
-  sig_catch(sig_cont, sig_cont_handler);
+  sig_catch(sig_cont, sig_int_handler);
   sig_block(sig_hangup);
+  sig_catch(sig_hangup, sig_int_handler);
   sig_block(sig_int);
-  sig_catch(sig_int, sig_int_handler);
+  sig_catch(sig_int, sig_cont_handler);
   sig_block(sig_pipe);
   sig_block(sig_term);
+  sig_catch(sig_term, sig_cont_handler);
 
   /* console */
   if ((ttyfd =open_write("/dev/console")) != -1) {
@@ -145,6 +147,8 @@ int main (int argc, const char * const *argv, char * const *envp) {
       sig_unblock(sig_child);
       sig_unblock(sig_cont);
       sig_unblock(sig_int);
+      sig_unblock(sig_term);
+      sig_unblock(sig_hangup);
 #ifdef IOPAUSE_POLL
       poll(&x, 1, 14000);
 #else
@@ -156,6 +160,8 @@ int main (int argc, const char * const *argv, char * const *envp) {
       sig_block(sig_cont);
       sig_block(sig_child);
       sig_block(sig_int);
+      sig_block(sig_term);
+      sig_block(sig_hangup);
       
       while (read(selfpipe[0], &ch, 1) == 1) {}
       while ((child =wait_nohang(&wstat)) > 0)
@@ -217,32 +223,14 @@ int main (int argc, const char * const *argv, char * const *envp) {
         sigc =sigi =0;
         continue;
       }
-      if (sigi && (stat(CTRLALTDEL, &s) != -1) && (s.st_mode & S_IXUSR)) {
-        strerr_warn2(INFO, "ctrl-alt-del request...", 0);
-        prog[0] =CTRLALTDEL; prog[1] =0;
-        while ((pid2 =fork()) == -1) {
-          strerr_warn4(FATAL, "unable to fork for \"", CTRLALTDEL,
-                       "\" pausing: ", &strerr_sys);
-          sleep(5);
-        }
-        if (!pid2) {
-          /* child */
-          strerr_warn3(INFO, "enter stage: ", prog[0], 0);
-          execve(*prog, (char *const *) prog, envp);
-          strerr_die4sys(0, FATAL, "unable to start child: ", prog[0], ": ");
-        }
-        if (wait_pid(&wstat, pid2) == -1)
-          strerr_warn2(FATAL, "wait_pid: ", &strerr_sys);
-        if (wait_crashed(wstat))
-          strerr_warn3(WARNING, "child crashed: ", CTRLALTDEL, 0);
-        strerr_warn3(INFO, "leave stage: ", prog[0], 0);
+      if (sigi) {
+    	strerr_warn2(INFO, "reload request...", 0);
+	    kill(pid, sig_hangup);
         sigi =0;
-        sigc++;
       }
-      if (sigc && (stat(STOPIT, &s) != -1) && (s.st_mode & S_IXUSR)) {
+      if (sigc) {
         int i;
         /* unlink(STOPIT); */
-        chmod(STOPIT, 0);
 
         /* kill stage 2 */
 #ifdef DEBUG
diff --git a/runit.h b/runit.h
index ba98386..2e05977 100644
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
+#define CTRLALTDEL "%INSTALL_DIR%/etc/runit/reload"
diff --git a/runsvdir.c b/runsvdir.c
index 07c1d8e..b26c873 100644
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
@@ -278,7 +278,8 @@ int main(int argc, char **argv) {
       _exit(0);
     case 2:
       for (i =0; i < svnum; i++) if (sv[i].pid) kill(sv[i].pid, SIGTERM);
-      _exit(111);
+      for (i =0; i < svnum; i++) if (sv[i].pid) waitpid(sv[i].pid, NULL, 0);
+      _exit(0);
     }
   }
   /* not reached */
diff --git a/sv.c b/sv.c
index 0125795..6cdcb86 100644
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
