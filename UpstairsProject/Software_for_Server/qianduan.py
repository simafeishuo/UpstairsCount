from flask import Flask, render_template, request, jsonify
import database

app = Flask(__name__)

# 全局变量，用于状态控制
global_register_status = "no"  # 当前是否在注册流程中


@app.route('/', methods=["POST", "GET"])
def login():
    """
    登录页面。
    """
    global global_register_status
    if global_register_status == 'yes':
        result = request.form.to_dict()
        try:
            database.into_user(result)  # 将注册信息写入数据库
            global_register_status = "no"
            return render_template('login.html', success="注册成功，请登录")
        except Exception as e:
            print(f"注册失败：{e}")
            return render_template('register.html', error="注册失败，请稍后重试")
    return render_template('login.html')


@app.route('/register', methods=['POST', "GET"])
def register():
    """
    注册页面。
    """
    global global_register_status
    global_register_status = "yes"
    return render_template('register.html')


@app.route('/home', methods=["POST", "GET"])
def home():
    """
    用户登录验证。
    """
    if request.method == 'POST':
        try:
            result = request.form.to_dict()  # 获取表单提交数据
            if not result.get('username') or not result.get('mima'):
                return render_template('page.html', error="用户名或密码不能为空")

            user_data = database.verfy_user(result)  # 验证用户信息
            if not user_data:
                return render_template('page.html', error="用户名或密码错误")

            # 登录成功，渲染主页
            user = user_data[0]
            return render_template('home.html', ID=user[0], user_name=user[1], age=user[3])
        except Exception as e:
            print(f"登录失败：{e}")
            return render_template('page.html', error="服务器发生错误，请稍后再试")
    return render_template('page.html')


@app.route('/suanfa', methods=['POST'])
def suanfa():
    """
    获取用户的阶段数据。
    """
    try:
        data = request.get_json()  # 获取 JSON 数据
        username = data.get('user_name')
        if not username:
            return jsonify({'error': '缺少 user_name 参数'}), 400
        data = database.get_stage_data(username)
        stage_data = data[0]
        mcu_stairs = data[1]
        mcu_floors = data[2]
        return jsonify({'result': stage_data,'mcu_stairs':mcu_stairs,'mcu_floors':mcu_floors})
    except Exception as e:
        print(f"获取阶段数据失败：{e}")
        return jsonify({'error': '服务器发生错误，请稍后再试'}), 500


@app.route('/export', methods=['GET'])
def export():
    """
    导出用户数据为 CSV 文件。
    """
    try:
        username = request.args.get('user_name')
        if not username:
            return jsonify({'error': '缺少 user_name 参数'}), 400

        database.export_data(username)  # 调用数据库导出功能
        return jsonify({'success': f"数据导出成功，文件名为 DATA_{username}.csv"})
    except Exception as e:
        print(f"导出数据失败：{e}")
        return jsonify({'error': '导出数据失败，请稍后再试'}), 500


@app.route('/admin', methods=['GET'])
def admin_get_data():
    """
    管理员获取所有用户的数据。
    """
    try:
        all_data = database.admin_get_data()
        return jsonify({'data': all_data})
    except Exception as e:
        print(f"获取管理员数据失败：{e}")
        return jsonify({'error': '获取数据失败，请稍后再试'}), 500


if __name__ == "__main__":
    app.run(host='127.0.0.1', port=5000, debug=True)
