// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-578039-Proxy_construct_prototype_change.js");
	}
}
function target() {};

var proxy = new Proxy(target, {
  get() {
    // Reset the initial map of the target.
    target.prototype = 123;
  }});

new proxy();
