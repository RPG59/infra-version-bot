FROM ghcr.io/userver-framework/ubuntu-userver-build-base:v1 as builder

WORKDIR /app

COPY . .

RUN mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j$((`nproc`-2)) 

FROM ghcr.io/userver-framework/ubuntu-userver-build-base:v1 as runner

WORKDIR /app

COPY --from=builder /app/build/service_template /app
COPY --from=builder /app/static-config.yaml /app
COPY --from=builder /app/dynamic_config.json /app

ENV GL_TOKEN=$GL_TOKEN
ENV GL_ORIGIN=$GL_ORIGIN

EXPOSE 8080
CMD [ "/app/service_template", "-c", "/app/static-config.yaml" ]



