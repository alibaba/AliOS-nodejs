// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MODULE

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker//modules-empty-import3.js");
	}
}
export {} from "modules-skip-empty-import.js";
import {counter} from "modules-skip-empty-import-aux.js";
assertEquals(1, counter);
