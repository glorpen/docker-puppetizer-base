#
# author: Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>
#

docker login -u "${DOCKER_USER}" -p "${DOCKER_PASSWORD}"

for tag in ${REPO_TAGS};
do
	name="${REPO_NAME}:${tag}"
	echo "Pushing ${name}"
	docker push "${name}"
done
