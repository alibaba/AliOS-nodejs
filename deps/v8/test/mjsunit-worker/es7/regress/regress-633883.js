// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es7/regress/regress-633883.js");
	}
}
v5 = new Array();
v17 = encodeURIComponent(v5);
v19 = isFinite();
v34 = new Array(v19);
v47 = v34.includes(v17);
