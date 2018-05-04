// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --harmony-dynamic-import

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/harmony/modules-import-10.js");
	}
}
var life;
import('modules-skip-6.js').then(namespace => life = namespace.life);

assertEquals(undefined, Object.life);

%RunMicrotasks();

assertEquals(42, Object.life);
assertEquals("42", life);
