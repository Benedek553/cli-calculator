FROM jetbrains/qodana-cpp:2025.3-eap

RUN apt-get update && \
    apt-get install -y libboost-all-dev qtbase5-dev  && \
    rm -rf /var/lib/apt/lists/*
