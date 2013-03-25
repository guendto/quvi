MAN1_TXT=\
  quvi.1.txt\
  quvi-dump.1.txt\
  quvi-get.1.txt\
  quvi-info.1.txt\
  quvi-scan.1.txt

DOC_MAN1=\
  quvi.1\
  quvi-dump.1\
  quvi-get.1\
  quvi-info.1\
  quvi-scan.1

MAN_TXT=$(MAN1_TXT)

ASCIIDOC_OPTS=\
  -f $(top_srcdir)/doc/asciidoc.conf\
  -a quvi_version="$(VERSION)"

if HAVE_A2X
$(DOC_MAN1):
	$(AM_V_GEN)$(A2X) -f manpage --asciidoc-opts="$(ASCIIDOC_OPTS)" \
		-D $(builddir) $(srcdir)/$@.txt
doc: $(DOC_MAN1)
else
doc:
	@echo configure did not find a2x
endif # HAVE_A2X

MOSTLYCLEANFILES=$(DOC_MAN1)
dist_man_MANS=$(DOC_MAN1)

EXTRA_DIST=\
  $(MAN_TXT)\
  common.txt\
  footer.txt\
  input.txt\
  opts-core-print-format.txt\
  opts-core.txt\
  opts-core-verbosity.txt\
  opts-exec.txt\
  opts-http.txt

# vim: set ts=2 sw=2 tw=72 expandtab:
