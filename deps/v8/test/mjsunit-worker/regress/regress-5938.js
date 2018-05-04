// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --lazy-inner-functions

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-5938.js");
	}
}
let global = 0;
{
  let confusing = 13;
  function lazy_func(b = confusing) { let confusing = 0; global = b; }
  lazy_func();
}

assertEquals(13, global);
