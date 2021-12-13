FROM jojoelfe/cistem_ci

RUN echo "deb [trusted=yes] http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/ /" >> /etc/apt/sources.list && apt-get update -y --allow-unauthenticated  && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends cuda-11-3 && rm -rf /var/lib/apt/lists/* && sed -i '$d' /etc/apt/sources.list 

