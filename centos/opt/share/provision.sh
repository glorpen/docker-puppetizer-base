set -e

OLD_REPO=/etc/yum.repos.d/temp-centos-old.repo

# setup CentosOs old repos
cat <<EOF> $OLD_REPO 
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
yum -y install puppet-agent puppetdb-termini

# prepare paths for puppetizer
mkdir -p /opt/puppetizer/sources /var/opt/puppetizer/{modules,vendor,hiera}
mkdir -p /etc/puppetizer.d/{services,scripts,health}
rm -rf /etc/puppetlabs/code/environments/production/*

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

# configure Bolt
mkdir -p ~/.puppetlabs

cat <<EOF> ~/.puppetlabs/bolt.yml
modulepath: "/var/opt/puppetizer/vendor/:/var/opt/puppetizer/modules/"
format: human
EOF

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
$OLD_REPO

find /opt/puppetlabs/ -iname "*.a" -delete
find /opt/puppetlabs/ -iname "*.so" -type f -exec strip -s {} \;
find /opt/puppetlabs/puppet/bin -type f -exec strip -s -v {} 2> /dev/null \;
