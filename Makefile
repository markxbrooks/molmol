# MOLMOL — build from repo root; run ./molmol (uses data/ via MOLMOLHOME).

.PHONY: all build clean install

all build:
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean

# Optional: place launcher next to data/ so argv[0] layout works without MOLMOLHOME
install: build
	ln -sf ../src/src/main/molmol data/molmol
	@echo "Installed symlink data/molmol -> build. Run: ./data/molmol"
	@echo "Or from repo root: ./molmol"
