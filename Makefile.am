ACLOCAL_AMFLAGS=-I m4
EXTRA_DIST=gen-ver.sh VERSION

SUBDIRS=po src doc

.PHONY: doc ChangeLog VERSION

VERSION:
	@if test -d "$(top_srcdir)/.git" -o -f "$(top_srcdir)/.git"; then \
		"$(top_srcdir)/gen-ver.sh" "$(top_srcdir)" > $@; \
	fi

ChangeLog:
	@if test -d "$(top_srcdir)/.git" -o -f "$(top_srcdir)/.git"; then \
		git log --stat --name-only --date=short --abbrev-commit > $@; \
	fi

doc:
	@make -C $(top_builddir)/doc/man1/ doc
	@make -C $(top_builddir)/doc/man5/ doc

dist-hook: ChangeLog VERSION
