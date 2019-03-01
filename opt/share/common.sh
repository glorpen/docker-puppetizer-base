#
# Author: Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>
#

set -e

puppetizer_root_dir="/opt/puppetizer"
puppetizer_bin="${puppetizer_root_dir}/bin"
#puppetizer_sources="/opt/puppetizer/sources"
puppetizer_var_dir="${puppetizer_root_dir}/puppet"
puppetizer_mods_dir="${puppetizer_var_dir}/modules"
#puppetizer_vendor_modules="/var/opt/puppetizer/vendor"
puppetizer_init="${puppetizer_var_dir}/init.pp"
puppet_conf_dir="${puppetizer_root_dir}/etc/puppet"
puppet_code_dir="${puppetizer_root_dir}/etc/code"
puppet_env_dir="${puppet_code_dir}/environments"
puppet_modules_dir="${puppetizer_var_dir}/modules-external"
puppetizer_modules_dir="${puppetizer_var_dir}/modules-internal"
#puppetizer_share_dir="/opt/puppetizer/share"
puppetizer_puppetfile="${puppet_conf_dir}/puppetfile"

puppetizer_health_dir="${puppetizer_root_dir}/health" #
puppetizer_initialized_token="${puppetizer_root_dir}/initialized" #

puppet_apply()
{
	env="${1:-production}"
	if [ "x$PUPPETIZER_DEBUG" == "xy" ] || [ "x${env}" == "xbuild" ];
	then
		debug_opts="--verbose --strict=warning"
	else
		debug_opts="--log_level warning"
	fi
	
	# apply puppet manifests and check for exit code
	set +e
	"${puppetizer_bin}/puppet" apply --detailed-exitcodes $debug_opts --environment=${env} "${puppetizer_init}"
	puppet_ret=$?
	set -e
	
	#return $puppet_ret
	
	if [ $puppet_ret == 2 ] || [ $puppet_ret == 0 ];
	then
		return 0
	else
		return 1
	fi
}

find_scripts()
{
	path="${1}"; shift
	find "${path}" -type f -perm -u+x $@ | sort
}

print_puppet_config()
{
	/opt/puppetizer/bin/puppet config print "${1}"
}

os_packages_init()
{
	%PUPPETIZER_PKG_INIT%
}

os_packages_cleanup()
{
	%PUPPETIZER_PKG_CLEANUP%
}

os_packages_install()
{
	%PUPPETIZER_PKG_INSTALL%
}
