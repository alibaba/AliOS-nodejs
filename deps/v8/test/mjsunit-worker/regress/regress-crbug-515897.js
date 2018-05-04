// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-515897.js");
	}
}
var r1 = new RegExp("\\\\/");
assertTrue(r1.test("\\/"));
var r2 = eval("/" + r1.source + "/");
assertEquals("\\\\\\/", r1.source);
assertEquals("\\\\\\/", r2.source);
