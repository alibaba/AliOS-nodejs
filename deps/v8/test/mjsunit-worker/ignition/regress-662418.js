// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/ignition/regress-662418.js");
	}
}
var valueof_calls = 0;

var v = {
  toString: function() {
    var z = w++;
  }
};
var w = {
  valueOf: function() {
    valueof_calls++;
  }
};
var x = { [v]: 'B' };
assertTrue(valueof_calls == 1);
