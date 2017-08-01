Feature: Project creation
  As someone collecting tasks
  I can create a project
  In order to organize my tasks

  Scenario: New projects appear in the list
    Given I display the available pages
    When I add a project named "Birthday" in the source named "TestData / Calendar1"
    And I list the items
    Then the list is:
       | display                                       | icon                |
       | Inbox                                         | mail-folder-inbox   |
       | Workday                                       | go-jump-today       |
       | Projects                                      | folder              |
       | Projects / Calendar1                          | folder              |
       | Projects / Calendar1 / Birthday               | view-pim-tasks      |
       | Projects / Calendar1 / Prepare talk about TDD | view-pim-tasks      |
       | Projects / Calendar1 / Read List              | view-pim-tasks      |
       | Projects / Calendar2                          | folder              |
       | Projects / Calendar2 / Backlog                | view-pim-tasks      |
       | Contexts                                      | folder              |
       | Contexts / Errands                            | view-pim-notes      |
       | Contexts / Online                             | view-pim-notes      |
