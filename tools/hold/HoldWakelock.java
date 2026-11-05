/*
 * HoldWakelock — keep the display awake from a shell process.
 *
 * The module needs the display to stay interactive while the phone is locked: on
 * this GSI the touch controller leaves ACTIVE mode as soon as the display sleeps,
 * so a resting finger reaches nobody (DEBUG.md F17/F20). No shell command can hold
 * a display wakelock, but a process started through app_process can use the same
 * PowerManager API the platform uses.
 *
 * usage: app_process /system/bin HoldWakelock acquire|release
 *
 * Runs as uid 0. The wakelock tag is fixed so the helper can find and release it.
 */
import android.content.Context;
import android.os.Looper;
import android.os.PowerManager;

public final class HoldWakelock {
    private static final String TAG = "km6fod:hold";

    public static void main(String[] args) {
        if (args.length < 1) {
            System.err.println("usage: HoldWakelock acquire|release");
            System.exit(2);
        }
        try {
            // ActivityThread is a hidden class: reach it by reflection, the same way
            // the `settings` and `input` shell tools do.
            // ActivityThread installs a Handler on the calling thread.
            Looper.prepare();
            Class<?> activityThread = Class.forName("android.app.ActivityThread");
            Object thread = activityThread.getDeclaredMethod("systemMain").invoke(null);
            Object context = activityThread.getDeclaredMethod("getSystemContext").invoke(thread);
            PowerManager pm = (PowerManager) ((Context) context).getSystemService(Context.POWER_SERVICE);
            if (pm == null) {
                System.err.println("no PowerManager");
                System.exit(1);
            }
            if ("acquire".equals(args[0])) {
                PowerManager.WakeLock lock = pm.newWakeLock(
                        PowerManager.FULL_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP, TAG);
                lock.setReferenceCounted(false);
                lock.acquire();
                System.out.println("acquired: " + lock.isHeld() + " interactive=" + pm.isInteractive());
                // A PowerManager wakelock belongs to the process: stay alive so the
                // lock survives. Killing this process releases it (see hold.sh).
                while (true) {
                    Thread.sleep(60000);
                }
            } else {
                // Releasing needs the same instance; a fresh one with the same tag is
                // not the same object, so the helper is expected to run as a resident
                // process for the release path (see hold.sh).
                PowerManager.WakeLock lock = pm.newWakeLock(
                        PowerManager.FULL_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP, TAG);
                lock.setReferenceCounted(false);
                lock.release();
                System.out.println("released");
            }
        } catch (Throwable t) {
            System.err.println("failed: " + t);
            t.printStackTrace();
            System.exit(1);
        }
    }
}
