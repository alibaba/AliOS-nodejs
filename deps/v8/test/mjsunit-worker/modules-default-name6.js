// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MODULE

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker//modules-default-name6.js");
	}
}
import {default as goo} from "modules-skip-default-name6.js";
assertEquals(
    {value: "default", configurable: true, writable: false, enumerable: false},
    Reflect.getOwnPropertyDescriptor(goo, 'name'));
