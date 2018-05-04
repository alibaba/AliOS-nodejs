// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Test that the parser throws on unmatched surrogates.
if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker//parse-surrogates.js");
	}
}
assertThrows("var \uD801\uABCD;", SyntaxError);
assertThrows("'\\u000\uD801\uABCD'", SyntaxError);
