#
# Copyright (c) 2006 Peter Postma <peter@pointless.nl>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

all:
	if [ -f autoop/Makefile ]; then cd autoop && $(MAKE); fi
	if [ -f autorejoin/Makefile ]; then cd autorejoin && $(MAKE); fi
	if [ -f dictionary/Makefile ]; then cd dictionary && $(MAKE); fi
	if [ -f feed/Makefile ]; then cd feed && $(MAKE) ; fi
	if [ -f google/Makefile ]; then cd google && $(MAKE); fi

clean:
	if [ -f autoop/Makefile ]; then cd autoop && $(MAKE) clean; fi
	if [ -f autorejoin/Makefile ]; then cd autorejoin && $(MAKE) clean; fi
	if [ -f dictionary/Makefile ]; then cd dictionary && $(MAKE) clean; fi
	if [ -f feed/Makefile ]; then cd feed && $(MAKE) clean; fi
	if [ -f google/Makefile ]; then cd google && $(MAKE) clean; fi

distclean:
	if [ -f autoop/Makefile ]; then cd autoop && $(MAKE) distclean; fi
	if [ -f autorejoin/Makefile ]; then cd autorejoin && $(MAKE) distclean; fi
	if [ -f dictionary/Makefile ]; then cd dictionary && $(MAKE) distclean; fi
	if [ -f feed/Makefile ]; then cd feed && $(MAKE) distclean; fi
	if [ -f google/Makefile ]; then cd google && $(MAKE) distclean; fi
