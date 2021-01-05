FROM hyperf/hyperf:7.4-alpine-v3.11-dev

LABEL maintainer="Swoole Team <team@swoole.com>" version="1.0" license="MIT"

COPY . /opt/www

WORKDIR /opt/www

RUN set -ex \
    && apk add boost-dev \
    && ln -sf /usr/bin/phpize7 /usr/local/bin/phpize \
    && ln -sf /usr/bin/php-config7 /usr/local/bin/php-config \
    && phpize \
    && ./configure \
    && make -s -j$(nproc) && make install \
    && echo "extension=yasd.so" > /etc/php7/conf.d/50_yasd.ini \
    # check
    && php -v \
    && php -m \
    && php --ri yasd \
    && echo -e "\033[42;37m Build Completed :).\033[0m\n"
