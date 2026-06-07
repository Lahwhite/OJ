USE myOJ;

INSERT INTO problem_users (username, role)
VALUES ('admin', 'admin');

INSERT INTO problems (
    title,
    description,
    difficulty,
    time_limit,
    memory_limit,
    input_description,
    output_description,
    sample_input,
    sample_output,
    created_by,
    is_public,
    submission_count,
    accepted_count
) VALUES (
    'Hello Problem',
    'This is a test problem for the problem management module.',
    'easy',
    1000,
    128,
    'Input a string.',
    'Output the same string.',
    'Hello',
    'Hello',
    1,
    TRUE,
    10,
    8
);

INSERT INTO test_cases (problem_id, input, output, is_sample, score)
VALUES
    (1, 'Hello\n', 'Hello\n', TRUE, 20),
    (1, 'World\n', 'World\n', FALSE, 80);

INSERT INTO tags (name, color, problem_count)
VALUES ('入门', '#1890ff', 1);

INSERT INTO problem_tags (problem_id, tag_id)
VALUES (1, 1);

INSERT INTO problem_user_status (
    user_id,
    problem_id,
    accepted,
    best_score,
    last_score,
    max_score,
    last_submitted_at,
    accepted_at
) VALUES (
    1,
    1,
    TRUE,
    100,
    100,
    100,
    CURRENT_TIMESTAMP,
    CURRENT_TIMESTAMP
);

select * from `myOJ`.`problem_users`;
select * from `myOJ`.`problems`;
select * from `myOJ`.`test_cases`;
select * from `myOJ`.`tags`;
select * from `myOJ`.`problem_tags`;
select * from `myOJ`.`problem_user_status`;
