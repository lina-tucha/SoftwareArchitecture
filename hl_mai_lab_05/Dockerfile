FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libpoco-dev \
    libpq-dev \
    postgresql-server-dev-all

COPY . /user
WORKDIR /user

RUN rm -rf build && mkdir build && cd build && cmake .. && make

EXPOSE 8080

CMD ["./build/social_network"]