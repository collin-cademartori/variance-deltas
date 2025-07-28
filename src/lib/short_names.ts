// short_name(["mu_b[1,1,2]", "mu_b[1,2,2]", "mu_b[1,3,3]","gamma[1]", "gamma[2]"])

export function short_name(
  name_set : string[],
  convert_greek : boolean = true,
  shorten_prefixes : boolean = true
) {
  const snames : string[] = [];
  const name_prefixes = new Set(name_set.map((name) => name.split("[")[0]));

  for(let np of name_prefixes) {
    const cur_names = name_set.filter((name) => name.startsWith(np));
    const indices = cur_names.map((name) => {
      const np = name.split("[");
      if(np.length == 1) {
        return([]);
      } else {
        return(np[1].substring(0, np[1].length - 1)
                    .split(",")
                    .map((s) => parseInt(s)));
      }
    });

    if(convert_greek) {
      for(const [gl, gu] of Object.entries(unicode_greek)) {
        np = np.replaceAll(gl, gu);
      }
    }

    if(shorten_prefixes) {
      np = np.split("_").map((s) => s[0]).join("");
    }

    snames.push(np + "[" + get_index_strs(indices).join(", ") + "]");
  }
  return(snames.join(", "));
}

export function by_parameter(param_strings : string[]) {
  const name_prefixes = [...new Set(param_strings.map((name) => name.split("[")[0]))];
  const params_by_name = name_prefixes.map((np) => {
    const cur_names = param_strings.filter((name) => name.startsWith(np));
    const indices = cur_names.map((name) => {
      const np = name.split("[");
      if(np.length == 1) {
        return([]);
      } else {
        return(np[1].substring(0, np[1].length - 1)
                    .split(",")
                    .map((s) => parseInt(s)));
      }
    });
    return({
      name : np,
      indices : indices
    })
  });
  return(params_by_name);
}

export function get_index_strs(indices : number[][], flatten = true) {

  if(indices.length === 0) {
    return([]);
  }

  const index_len = indices[0].length;

  // Calculate which indices are non-constant
  const var_i : number[] = [];
  for(let ii = 0; ii < index_len; ++ii) {
    const ind_i = indices.map((ind) => ind[ii]);
    if(!const_arr(ind_i)) {
      var_i.push(ii);
    }
  }

  let index_strings : string[] = [];
  if(var_i.length > 1) {
    // If more than one dimension varies, reduce to raw notation
    if(flatten) {
      let index_string = indices.reduceRight(
        (p, n) => "[" + n.join(",") + "]," + p,
      "");
      index_string = index_string.substring(0, index_string.length - 1);
      index_strings[0] = index_string
    } else {
      index_strings = indices.reduceRight(
        (p, n) => p.map((pv, pi) => pv + ", " + n[pi].toString()),
      rep_array("", index_len));
    }
    //const sname = np + "[" + index_string + "]";
    //snames.push(sname);
  } else if(is_seq(indices.map((i) => i[var_i[0]]))) {
    const var_vals = indices.map((i) => i[var_i[0]]).toSorted((a,b) => a - b);
    const br = var_vals[0];
    const er = var_vals[var_vals.length - 1];
    index_strings = indices[0].map(
      (iv, j) => j == var_i[0] ? (br + "-" + er) : iv.toString()
    )
    // const sname = np + "[" + 
    //   indices[0].map(
    //     (iv, j) => j == var_i[0] ? (br + "-" + er) : iv.toString()
    //   ).join(",") + "]";  
    // snames.push(sname);
  } else {
    const var_vals = indices.map((i) => i[var_i[0]]).toSorted((a,b) => a - b);
    let vv_str = var_vals.join(",");
    if(indices[0].length > 1) {
      vv_str = "[" + vv_str + "]";
    }
    index_strings = indices[0].map(
      (iv, j) => j == var_i[0] ? vv_str : iv.toString()
    )
    // const sname = np + "[" + 
    //   indices[0].map(
    //     (iv, j) => j == var_i[0] ? vv_str : iv.toString()
    //   ).join(",") + "]";
    // snames.push(sname);
  }
  return(index_strings);
}

export function parse_index_strs(index_strs : string[]) {
  const long_indices = index_strs.map((is) => {
    if(is.includes("-")) {
      const range_strs = is.split("-");
      const range_nums = range_strs.map((i) => parseInt(i));
      if(range_strs.length === 2 && !isNaN(range_nums[0]) && !isNaN(range_nums[1]) && range_nums[1] >= range_nums[0]) {
        return(seq_array(range_nums[0], range_nums[1]));
      } else {
        throw new Error("Cannot parse range!");
      }
    } else {
      const index_strs = is.split(",");
      const index_nums = index_strs.map((i) => parseInt(i));
      if(index_nums.reduce((p, n) => p && !isNaN(n), true)) {
        return(index_nums)
      } else {
        throw new Error("Cannot parse list!");
      }
    }
  });

  if(!const_arr(long_indices.map((li) => li.length).filter((len) => len > 1))) {
    throw new Error("Dimensions unequal length!");
  }

  const indices = [];
  for(let ii = 0; ii < long_indices[0].length; ++ii) {
    indices.push(long_indices.map((li) => li.length == 1 ? li[0] : li[ii]));
  }

  return(indices);
}

export function rep_array(s : string | number | boolean, n :number) {
  return((new Array(n)).fill(s));
} 

function const_arr(arr : number[]) {
  return(arr.reduce((p, n) => p && (n == arr[0]), true));
}

function is_seq(arr : number[]) {
  const arrs = arr.sort((a,b) => a - b);
  return(arrs.length == (1 + arrs[arrs.length - 1] - arrs[0]) && arrs.length > 1);
}

function seq_array(lower : number, upper : number) {
  return([...(new Array(upper - lower + 1)).keys().map((k) => k + lower)]);
}

const unicode_greek = {
  'alpha': '\u{03B1}',
  'beta': '\u{03B2}',
  'gamma': '\u{03B3}',
  'delta': '\u{03B4}',
  'epsilon': '\u{03B5}',
  'zeta': '\u{03B6}',
  'theta': '\u{03B8}',
  'iota': '\u{03B9}',
  'kappa': '\u{03BA}',
  'lambda': '\u{03BB}',
  'mu': '\u{03BC}',
  'nu': '\u{03BD}',
  'xi': '\u{03BE}',
  'omicron': '\u{03BF}',
  'rho': '\u{03C1}',
  'sigma': '\u{03C3}',
  'tau': '\u{03C4}',
  'upsilon': '\u{03C5}',
  'phi': '\u{03C6}',
  'chi': '\u{03C7}',
  'psi': '\u{03C8}',
  'omega': '\u{03C9}',
  'pi': '\u{03C0}',
  'eta': '\u{03B7}'
}