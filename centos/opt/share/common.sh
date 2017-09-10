#
# Author: Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>
#

set -e

puppetizer_sources="/opt/puppetizer/sources"
puppetizer_modules="/var/opt/puppetizer/modules"
vendor_modules="/var/opt/puppetizer/vendor"
puppetizer_init="/var/opt/puppetizer/init.pp"

function setup_puppetizer_modules()
{
	echo "" > "${puppetizer_init}"

	cat << EOF > /etc/puppetlabs/puppet/hiera.yaml	
---
# Hiera 5 Global configuration file

version: 5

hierarchy:
  - name: Puppetizer runtime data
    data_hash: yaml_data
    path: /var/opt/puppetizer/hiera/runtime.yaml
EOF
	
	find "${puppetizer_sources}" -maxdepth 1 -mindepth 1 -type d | while read n;
	do
		short_name="$(basename $n)"
		module_name="puppetizer_${short_name}"
		ln -fs "${n}/code" "${puppetizer_modules}/${module_name}";
		echo "include ::${module_name}" >> "${puppetizer_init}"
		
		cat << EOF >> /etc/puppetlabs/puppet/hiera.yaml
  - name: Puppetizer env data for ${module_name}
    data_hash: yaml_data
    path: /opt/puppetizer/sources/${short_name}/hiera/env-%{environment}.yaml
  - name: Puppetizer data for ${module_name}
    data_hash: yaml_data
    path: /opt/puppetizer/sources/${short_name}/hiera/puppetizer.yaml
EOF
		
	done
}

function puppet_apply()
{
	env="${1:-production}"
	
	# apply puppet manifests and check for exit code
	set +e
	/opt/puppetlabs/bin/puppet apply --detailed-exitcodes --verbose --environment=${env} --modulepath="${puppetizer_modules}:${vendor_modules}" "${puppetizer_init}"
	puppet_ret=$?
	set -e
	
	if [ $puppet_ret != 2 ];
	then
		return 1
	fi
	
	return 0
}
