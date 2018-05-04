// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var k = 0; k < ThreadWorkerCount; k++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-707187.js");
	}
}
let i = 0;
let re = /./g;
re.exec = () => {
  if (i++ == 0) return { length: 2147483648 };
  return null;
};

"".replace(re);
