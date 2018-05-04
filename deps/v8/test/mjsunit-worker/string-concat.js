// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker//string-concat.js");
	}
}
function Stringified(toString) {
  var valueOf = "-" + toString + "-";
  return {
    toString: function() { return toString; },
    valueOf: function() { return valueOf; }
  };
}

assertEquals("a.b.", "a.".concat(Stringified("b.")));
assertEquals("a.b.c.", "a.".concat(Stringified("b."), Stringified("c.")));
