#!/usr/bin/env bash
#
# Builds a Windows installer on the Windows machine, over ssh, and leaves it published.
#
# Runs on Linux: it is the caller, not the thing that builds.
#
#   ./build_sptk_windows_ssh.bat            both, SPTK first because XMQ links against it
#   ./build_sptk_windows_ssh.bat sptk5      one of them
#   ./build_sptk_windows_ssh.bat xmq
#
# The machine is alexe@10.1.1.206 (WIN-DEV2), which is the one that stays on - that is what makes
# an unattended run possible. WIN-DEV1 at .203 is started on demand; set WIN_HOST to use it, or any
# other. The branch built is
# whatever SPTK_VERSION and XMQ_VERSION say here, so a release is driven by editing those two
# files, not by editing this.
#
# It is meant to be safe to leave running overnight: each package is a separate ssh, its output is
# kept, and a failure stops the run rather than going on to the next one and burying which of them
# broke.

set -uo pipefail

cd "$(dirname "$0")" || exit 1

host=${WIN_HOST:-alexe@10.1.1.206}
log_dir=${LOG_DIR:-/tmp/windows-build-$(date +%Y-%m-%d-%H%M)}
mkdir -p "$log_dir"

packages=("$@")
if [ ${#packages[@]} -eq 0 ]; then
    packages=(sptk5 xmq)
fi

# Windows paths, and they have to survive both this shell and the remote one, so single quotes
# here and no escaping there.
sptk_repo='C:\workspace\sptk5'
xmq_repo='C:\workspace\xmq'
scripts_dir='C:\workspace\sptk5\build.scripts'

failed=0
for requested in "${packages[@]}"; do
    case "${requested,,}" in
        sptk5|sptk) package=sptk5; repo=$sptk_repo; branch=$(cat SPTK_VERSION) ;;
        xmq)        package=XMQ;   repo=$xmq_repo;  branch=$(cat XMQ_VERSION) ;;
        *) echo "Unknown package '$requested'. Use sptk5 or xmq." >&2; exit 1 ;;
    esac

    log="$log_dir/$package.log"
    echo "$(date +%H:%M:%S) $package: branch $branch on $host, log $log"

    # The branch is set here rather than in the Windows script, which pulls but does not know
    # which branch a release is on. reset --hard first: a build tree that someone has been
    # debugging in will otherwise stop the checkout, and nothing on that machine is authored.
    if ! ssh "$host" "cmd /c \"cd /d $repo && git reset --hard && git fetch --prune && git checkout $branch && git pull --ff-only\"" > "$log" 2>&1; then
        echo "$(date +%H:%M:%S) $package: could not put $repo on $branch - see $log" >&2
        failed=1
        break
    fi

    if ! ssh "$host" "cmd /c \"cd /d $scripts_dir && build_sptk_windows.bat $package\"" >> "$log" 2>&1; then
        echo "$(date +%H:%M:%S) $package: build failed - see $log" >&2
        tail -20 "$log" >&2
        failed=1
        break
    fi

    # The Windows script says what it published; repeat it here so a whole run reads as one story.
    grep -E "^Published |^Installer " "$log" | tail -2
    echo "$(date +%H:%M:%S) $package: done"
done

exit $failed
