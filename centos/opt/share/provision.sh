#
# Author: Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>
#

set -e

CENTOS_OLD_REPO=/etc/yum.repos.d/temp-centos-old.repo

USE_BOLT=no
USE_PUPPETDB=no
SYSTEM=unknown

PUPPETIZER_BIN=/opt/puppetizer/bin

for opt in "$@";
do
	key="$(echo "${opt}" | sed -e 's/=.*//')"
	val="$(echo "${opt}" | sed -e 's/.*=//')"
	
	case "$key" in
		bolt|puppetdb)
			export USE_$(echo "$key" | tr '[:lower:]' '[:upper:]')="$val";;
		system)
			export SYSTEM="$val";;
		*)
			echo "Unknown option \"$key\""
			exit 1;
		;;
	esac
done

provision_puppet_centos(){
	# setup CentosOs old repos
	cat <<EOF> $CENTOS_OLD_REPO 
[old-base]
name=CentOS-Old-7.4.1708 - Base
baseurl=http://vault.centos.org/7.3.1611/os/\$basearch/
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-CentOS-7

[old-updates]
name=CentOS-Old-7.4.1708 - Updates
baseurl=http://vault.centos.org/7.3.1611/updates/\$basearch/
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-CentOS-7
EOF
	
	# install puppet repo and puppet agent
	yum -y install https://yum.puppetlabs.com/puppet5/puppet5-release-el-7.noarch.rpm
	yum -y install puppet-agent
	
	ln -s /opt/puppetlabs/bin/puppet "${PUPPETIZER_BIN}/puppet"
}

provision_puppet_alpine(){
	PUPPET_VERSION="5.3.2"
	FACTER_VERSION="2.5.1"
	
	apk add --update \
      ca-certificates  pciutils \
      ruby ruby-irb ruby-rdoc
    echo http://dl-4.alpinelinux.org/alpine/edge/community/ >> /etc/apk/repositories
    apk add --update shadow
    gem install puppet:"$PUPPET_VERSION" facter:"$FACTER_VERSION"
	#/usr/bin/puppet module install puppetlabs-apk
	
	# Workaround for https://tickets.puppetlabs.com/browse/FACT-1351
	rm /usr/lib/ruby/gems/*/gems/facter-"$FACTER_VERSION"/lib/facter/blockdevices.rb
	
	/usr/bin/puppet module install puppetlabs-apk
	mkdir -p /etc/puppetlabs/code/environments/production /etc/puppetlabs/puppet
	
	ln -s /usr/bin/puppet "${PUPPETIZER_BIN}/puppet"
}

provision_puppet(){
	# prepare paths for puppetizer
	mkdir -p /opt/puppetizer/{sources,bin} \
		/var/opt/puppetizer/{modules,vendor,hiera} \
		/var/opt/puppetizer/{services,scripts,health}
	
	provision_puppet_${SYSTEM}
	
	rm -rf /etc/puppetlabs/code/environments/production/*
}

provision_bolt_centos(){
	# Install Puppet Bolt
	yum install -y gcc-4.8.5-16.el7 glibc-devel-2.17-196.el7 make libffi-devel -x glibc,glibc-common,libgcc
	/opt/puppetlabs/puppet/bin/gem install bolt
	yum remove -y gcc make cpp glibc-devel kernel-headers libgomp mpfr libmpc
	
	# fix libffi-devel error when removing
	yum remove -y libffi-devel || ( \
	    echo -e 'START-INFO-DIR-ENTRY\n* libffi: (libffi).\nEND-INFO-DIR-ENTRY' | gzip > /usr/share/info/libffi.info.gz \
		&& yum remove -y libffi-devel \
		&& rm /usr/share/info/libffi.info.gz \
	)
	
	ln -s /opt/puppetlabs/puppet/bin/bolt "${PUPPETIZER_BIN}/bolt"
}

provision_bolt_alpine(){
	apk add gcc make ruby-dev libffi-dev musl-dev
	gem install -N bolt
	apk del gcc make ruby-dev libffi-dev musl-dev
	#TODO: check for leftover files
	
	ln -s /usr/bin/bolt "${PUPPETIZER_BIN}/bolt"
}

provision_bolt(){
	provision_bolt_${SYSTEM}
	
	# configure Bolt
	mkdir -p ~/.puppetlabs
	
	cat <<EOF> ~/.puppetlabs/bolt.yml
modulepath: "/var/opt/puppetizer/vendor/:/var/opt/puppetizer/modules/"
format: human
EOF
}

provision_puppetdb_centos(){
	yum install -y puppetdb-termini
}
provision_puppetdb_alpine(){
	gem install -N puppetdb-terminus
}

provision_puppetdb(){
	provision_puppetdb_${SYSTEM}
}

cleanup_alpine(){
	rm -rf /var/cache/apk/*
}

cleanup_centos(){
	yum clean all
	
	rm -rf \
	/var/lib/yum/yumdb /var/cache/yum \
	/opt/puppetlabs/puppet/lib/ruby/gems/*/doc \
	/opt/puppetlabs/puppet/share/man/* \
	/opt/puppetlabs/puppet/ssl/man/* \
	/opt/puppetlabs/puppet/include \
	/opt/puppetlabs/puppet/lib/ruby/gems/*/cache/ \
	/var/tmp/* \
	/var/lib/yum/history/* /var/lib/yum/repos/x86_64/7/old-* \
	/root/.gem /root/.pki \
	$CENTOS_OLD_REPO
	
	find /opt/puppetlabs/ -iname "*.a" -delete
	find /opt/puppetlabs/ -iname "*.so" -type f -exec strip -s {} \;
	find /opt/puppetlabs/puppet/bin -type f -exec strip -s -v {} 2> /dev/null \;
}

cleanup(){
	cleanup_${SYSTEM}
}

provision_puppet
[ "$USE_PUPPETDB" == "yes" ] && provision_puppetdb
[ "$USE_BOLT" == "yes" ] && provision_bolt
cleanup
