if(!userInfo) return cb("无权限访问");

	

	var authFn = function(rid,keyRolePermissions,cb) {
		permissionAPIDAO.list(function(err,permissions){
			if(err) return cb("获取权限数据失败");
			var keyPermissions = _.keyBy(permissions,'ps_id');
			var rootPermissionsResult = {};
			// 处理一级菜单
			for(idx in permissions) {

				permission = permissions[idx];
				
				if(permission.ps_level == 0) {
					if(rid != 0) {
						if(!keyRolePermissions[permission.ps_id]) continue;;
					}
					rootPermissionsResult[permission.ps_id] = {
						"id":permission.ps_id,
						"authName":permission.ps_name,
						"path":permission.ps_api_path,
						"children":[],
						"order":permission.ps_api_order
					};
				}
			}

			// 处理二级菜单
			for(idx in permissions) {
				permission = permissions[idx];
				if(permission.ps_level == 1) {
					if(rid != 0) {
						if(!keyRolePermissions[permission.ps_id]) continue;;
					}
					parentPermissionResult = rootPermissionsResult[permission.ps_pid];
					if(parentPermissionResult) {
						parentPermissionResult.children.push({
							"id":permission.ps_id,
							"authName":permission.ps_name,
							"path":permission.ps_api_path,
							"children":[],
							"order":permission.ps_api_order
						});
					}
				}
			}
			// 排序
			result = _.values(rootPermissionsResult);
			result = _.sortBy(result,"order");
			for(idx in result) {
				subresult = result[idx];
				subresult.children = _.sortBy(subresult.children,"order");
			}

			cb(null,result);
		});
	}
