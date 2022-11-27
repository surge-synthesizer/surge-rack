# How to do a new install to the library

Basically you need to

1. Make sure CI builds
2. Potentially check in the docker image (although CI *should* cover that)
3. Make a branch
4. Update plugin.json in the branch
4. Tag, Commit and Push atomically
5. cancel the release branch build
6. Submit to the library

So here's how

## Make sure CI builds and Check in Docker

CI is easy. Just you know, merge.

The docker image can be run with `./scripts/drun.sh` which maps paths.
If you aren't baconpauls particular ubuntu image you may need to modify
some of the paths. One in the image, if you keep the envs and paths set
up, you can do `make plugin-linux-build` and it will do a toolchain
equivalent build

## Branch, update plugin.json, Tag, commit, Push atomically

We are using branches `releases-xt/v(version)` and tag `v(version)` so

```bash
git checkout -b releases-xt/v2.0.3.0
vi plugin.json  # Edit the 'version' to match
git add plugin.json
# you may want to cherry pick other items in or out here
git commit -m "2.0.3.0 release branch"
git tag v2.0.3.0 -m "2.0.3.0 release tag"
git push --atomic upstream-write releases-xt/v2.0.3.0 v2.0.3.0
```

## Cancel one of the github actions builds associated with the branch

Our CI actions run on a push of a tag or a branch, but only make a
release on the tag, so that branch will spanner the nightly. We
should fix this but for now, just go to the actions tab on github
and cancel the run which is happening for `releases-xt/v2.0.3.0` but
not the one for `v2.0.3.0`. Soon enough a release build will appear

## Submit to the library

```
RACK_DIR=../R2X86/Rack-SDK make issue_blurb
```

that will make a blurb which you can paste into https://github.com/VCVRack/community/issues/745
and then the machinery will start churning.