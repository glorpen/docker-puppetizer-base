#
# author: Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>
#

set -e

source ci/lib.sh

for tag_path in $(list_tags);
do
	docker build -t "${REPO_NAME}":"$(echo "${tag_path}" | cut -d: -f2)" -f "$(echo "${tag_path}" | cut -d: -f1)" .
done
