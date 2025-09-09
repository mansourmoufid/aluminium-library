.PHONY: wheel
wheel:
	cd .. && "$(MAKE)" -f darwin/Makefile so
	cp -f ../build/arm64-apple-darwin/libal.dylib libal/libal.so
	python setup.py bdist_wheel --plat-name=macosx_11_0_arm64

.PHONY: check-wheel
check-wheel:
	unzip -l dist/aluminium_library-*.whl | grep libal.so && exit 0

.PHONY: clean-wheel
clean-wheel:
	rm -f libal/libal.so
	rm -f dist/aluminium_library-*.whl

.PHONY: sdist
sdist:
	python setup.py sdist

.PHONY: check-sdist
check-sdist:
	gunzip -c dist/aluminium-library-*.tar.gz | tar -f - -t | grep libal.so && exit 1 || exit 0

.PHONY: clean-sdist
clean-sdist:
	rm -f dist/aluminium-library-*.tar.gz

.PHONY: cleanup
cleanup:
	rm -rf build
	rm -rf *.egg-info

.PHONY: clean
clean: cleanup clean-wheel
	rm -rf dist
