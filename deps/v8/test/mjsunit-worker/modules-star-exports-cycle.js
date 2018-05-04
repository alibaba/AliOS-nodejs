// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MODULE

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker//modules-star-exports-cycle.js");
	}
}
const bar = 42;
export {bar as foo};

import {foo} from "modules-skip-star-exports-cycle.js";
assertEquals(42, foo);
