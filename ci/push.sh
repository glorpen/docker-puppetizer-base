#
# author: Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>
#

docker login -u "${DOCKER_USER}" -p "${DOCKER_PASSWORD}"

source ci/lib.sh

for tag in $(list_tags_only);
do
	name="${REPO_NAME}:${tag}"
	echo "Pushing ${name}"
	docker push "${name}"
done
