cmake --preset=release && \
cd build-release && ninja && cd .. && \
cp build-release/snippy `which snippy`
