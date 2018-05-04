// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Environment Variables: LC_ALL=pt-BR.UTF8

// The data files packaged with d8 currently have Brazillian Portugese
// DateTimeFormat but not Collation

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-6288.js");
	}
}
if (this.Intl) {
  assertEquals('und', Intl.Collator().resolvedOptions().locale);
  assertEquals('pt-BR', Intl.DateTimeFormat().resolvedOptions().locale);
}
