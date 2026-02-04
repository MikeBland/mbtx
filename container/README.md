Modern Linux systems may have compile toolchains that are too new to
successfully build the software in this repository. The `Containerfile` in this
directory will produce a container image with the necessary toolchain for
compiling `ersky9x`.

## Build the container image

To build the container image, from the top level of the repository run:

```
podman build -t ersky-builder container
```

Or:

```
docker build -t ersky-builder -f container/Containerfile container
```

## Compile the software

From the top level of the repository, run:

```
podman run --rm -v $PWD:/src:z -w /src/radio/ersky9x/src ersky-builder make
```

Or:

```
docker run --rm -v $PWD:/src:z -w /src/radio/ersky9x/src ersky-builder make
```

You will find the compiled binary in `./radio/ersky9x/src`.
