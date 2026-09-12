cmake --preset=release && \
cd build-release && ninja && cd .. && \
mkdir -p /pub_data/installed && \
cp build-release/snippy /pub_data/installed/snippy && \
cp build-release/snippy-qml /pub_data/installed/snippy-qml
