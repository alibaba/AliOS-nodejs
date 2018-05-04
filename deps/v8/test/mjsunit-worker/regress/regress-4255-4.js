// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-4255-4.js");
	}
}
'use strict';
class C { constructor() {} }
// Trigger OSR
for (var i = 0; i < 1000000; i++);
