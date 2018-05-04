// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/regress/regress-3938.js");
	}
}
'use strict';

assertThrows(function() { for (const i = 0; ; i++) {} }, TypeError);
assertThrows("'use strict'; for (const i = 0; ; i++) {}", TypeError);
