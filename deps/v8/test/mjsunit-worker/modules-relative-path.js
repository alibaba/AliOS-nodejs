// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MODULE

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker//modules-relative-path.js");
	}
}
import {x as y} from "./modules-relative-path.js";
export let x = 0;

assertEquals(0, x);
assertEquals(x, y);
x++;
assertEquals(1, x);
assertEquals(x, y);
