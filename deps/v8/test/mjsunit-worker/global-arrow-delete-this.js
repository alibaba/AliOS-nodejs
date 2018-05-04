// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Make sure that we correctly resolve this when compiling an arrow function in
// a with scope in an arrow function.
if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker//global-arrow-delete-this.js");
	}
}
a = () => {
  let x
  with ({}) x = () => { "use strict"; delete this }
  return x
}
a()()


// Make sure that we correctly resolve this when compiling a program in an arrow
// function.
a = ()=>eval('"use strict"; delete this')
a()
