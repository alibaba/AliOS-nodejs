// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --harmony-dynamic-import

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/harmony/modules-import-5.js");
	}
}
var life;
let x = 'modules-skip-1.js';
import(x).then(namespace => life = namespace.life());
x = 'modules-skip-2.js';

%RunMicrotasks();
assertEquals(42, life);
